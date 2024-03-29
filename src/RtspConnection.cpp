#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "Logging.h"
#include "RtspConnection.h"

static void getPeerIp(int sockFd, std::string& ip)
{
    struct sockaddr_in addr;
    socklen_t addrLen = sizeof(struct sockaddr_in);
    getpeername(sockFd, (struct sockaddr*)&addr, &addrLen);
    ip = inet_ntoa(addr.sin_addr);
}

RtspConnection* RtspConnection::createNew(RtspServer* rtspServer, int sockFd)
{
    return new RtspConnection(rtspServer, sockFd);
}

RtspConnection::RtspConnection(RtspServer* rtspServer, int sockFd)
    : TcpConnection(rtspServer->getEnv(), sockFd),
    mRtspServer(rtspServer),
    mMethod(NONE),
    mTrackId(MediaSession::TrackIdNone),
    mSessionId(rand()),
    mIsRtpOverTcp(false)
{
    for(int i = 0; i < MEDIA_MAX_TRACK_NUM; ++i)
    {
        mRtpInstances[i] = NULL;
        mRtcpInstances[i] = NULL;
    }
    getPeerIp(sockFd, mPeerIp);
}

RtspConnection::~RtspConnection()
{
    for(int i = 0; i < MEDIA_MAX_TRACK_NUM; ++i)
    {
        if(mRtpInstances[i])
        {
            if(mSession)
                mSession->removeRtpInstance(mRtpInstances[i]);
            delete mRtpInstances[i];
        }

        if(mRtcpInstances[i])
            delete mRtcpInstances[i];
    }
}

void RtspConnection::handleReadBytes()
{
    if(mIsRtpOverTcp)
    {
        if(*mInputBuffer.peek() == '$')
        {
            handleRtpOverTcp();
            return ;
        }
    }

    int ret = parseRequest();
    if(!ret)
    {
        LOG_WARNING("Failed to parse request\n");
        goto err;
    }

    switch (mMethod)
    {
    case OPTIONS:
        if(!handleCmdOption())
            goto err;
        break;
    case DESCRIBE:
        if(!handleCmdDescribe())
            goto err;
        break;
    case SETUP:
        if(!handleCmdSetup())
            goto err;
        break;
    case PLAY:
        if(!handleCmdPlay())
            goto err;
        break;
    case TEARDOWN:
        if(!handleCmdTeardown())
            goto err;
        break;
    case GET_PARAMETER:
        if(!handleCmdGetParameter())
            goto err;
        break;
    default:
        goto err;
        break;
    }

    return ;
err:
    handleDisconnection();
}

bool RtspConnection::parseRequest()
{
    std::cout << std::string(mInputBuffer.peek(), mInputBuffer.beginWrite()) << std::endl;
    const char* crlf = mInputBuffer.findCRLF();
    if(crlf == NULL)
    {
        mInputBuffer.retrieveAll();
        return false;
    }

    bool ret = parseRequest1(mInputBuffer.peek(), crlf);
    if(!ret)
    {
        mInputBuffer.retrieveAll();
        return false;
    }
    mInputBuffer.retrieveUntil(crlf + 2);

    /* 处理非请求行的其它内容 */
    crlf = mInputBuffer.findLastCRLF();
    if(crlf == NULL)
    {
        mInputBuffer.retrieveAll();
        return false;
    }

    ret = parseRequest2(mInputBuffer.peek(), crlf);
    if(!ret)
    {
        mInputBuffer.retrieveAll();
        return false;
    }
    mInputBuffer.retrieveUntil(crlf + 2);
    return true;
}

bool RtspConnection::parseRequest1(const char* begin, const char* end)
{
    std::string message(begin, end);
    char method[64] = {0};
    char url[512] = {0};
    char version[64] = {0};

    if(sscanf(message.c_str(), "%s %s %s", method, url, version) != 3)
        return false;
    
    if(!strcmp(method, "OPTIONS"))
        mMethod = OPTIONS;
    else if(!strcmp(method, "DESCRIBE"))
        mMethod = DESCRIBE;
    else if(!strcmp(method, "SETUP"))
        mMethod = SETUP;
    else if(!strcmp(method, "PLAY"))
        mMethod = PLAY;
    else if(!strcmp(method, "TEARDOWN"))
        mMethod = TEARDOWN;
    else if(!strcmp(method, "GET_PARAMETER"))
        mMethod = GET_PARAMETER;
    else
    {
        mMethod = NONE;
        return false;
    }

    if(strncmp(url, "rtsp://", 7) != 0)
        return false;
    
    uint16_t port = 0;
    char ip[64] = {0};
    char suffix[64] = {0};

    if(sscanf(url + 7, "%[^:]:%hu/%s", ip, &port, suffix) == 3)
    {}
    else if(sscanf(url + 7, "%[^/]/%s", ip, suffix) == 2)
        port == 554;
    else
        return false;

    mUrl = url;
    mSuffix = suffix;

    return true;
}

bool RtspConnection::parseCSeq(std::string& message)
{
    size_t pos = message.find("CSeq");
    if(pos != std::string::npos)
    {
        uint32_t cseq = 0;
        sscanf(message.c_str() + pos, "%*[^:]: %u", &cseq);
        mCSeq = cseq;
        return true;
    }
    return false;
}

// describe
bool RtspConnection::parseAccept(std::string& message)
{
    if(message.rfind("Accept") == std::string::npos || message.find("sdp") == std::string::npos)
        return false;
    return true;
}

bool RtspConnection::parseTransport(std::string& message)
{
    size_t pos = message.find("Transport");
    if(pos != std::string::npos)
    {
        if((pos = message.find("RTP/AVP/TCP")) != std::string::npos )
        {   
            // 通过TCP进行Rtp内容传输
            uint8_t rtpChannel, rtcpChannel;
            mIsRtpOverTcp = true;

            if(sscanf(message.c_str() + pos, "%*[^;];%*[^;];%*[^=]=%hhu-%hhu", &rtpChannel, &rtcpChannel) != 2)
                return false;
            
            mRtpChannel = rtpChannel;

            return true;
        }
        else if((pos = message.find("RTP/AVP")) != std::string::npos)
        {
            // 通过UDP进行传输
            uint16_t rtpPort = 0, rtcpPort = 0;
            if(message.find("unicast", pos) != std::string::npos)
            {
                if(sscanf(message.c_str() + pos, "%*[^;];%*[^;];%*[^=]=%hu-%hu", &rtpPort, &rtcpPort) != 2)
                    return false;
            }
            else if(message.find("multicast", pos) != std::string::npos)
                return true;
            else 
                return false;
            
            mPeerRtpPort = rtpPort;
            mPeerRtcpPort = rtcpPort;
        }
        else 
            return false;

        return true;
    }
    return false;
}

bool RtspConnection::parseMediaTrack()
{
    size_t pos = mUrl.find("track0");
    if(pos != std::string::npos)
    {
        mTrackId = MediaSession::TrackId0;
        return true;
    }

    pos = mUrl.find("track1");
    if(pos != std::string::npos)
    {
        mTrackId = MediaSession::TrackId1;
        return true;
    }
    return false;
}

bool RtspConnection::parseSessionId(std::string& message)
{
    size_t pos = message.find("Session");
    if(pos != std::string::npos)
    {
        uint32_t sessionId = 0;
        if(sscanf(message.c_str() + pos, "%*[^:]: %u", &sessionId) != 1)
            return false;
        return true;
    }
    return false;   
}

bool RtspConnection::parseRequest2(const char* begin, const char* end)
{
    std::string message(begin, end);

    if(!parseCSeq(message))
        return false;
    
    if(mMethod == OPTIONS)
        return true;
    
    if(mMethod == DESCRIBE)
        return parseAccept(message);

    if(mMethod == SETUP)
    {
        if(!parseTransport(message))
            return false;
        return parseMediaTrack();
    }

    if(mMethod == PLAY)
        return parseSessionId(message);

    if(mMethod == TEARDOWN)
        return true;

    if(mMethod == GET_PARAMETER)
        return true;

    return false;
}

bool RtspConnection::handleCmdOption()
{
    snprintf(mBuffer, sizeof(mBuffer), 
            "RTSP/1.0 200 OK\r\n"
            "CSeq: %u\r\n"
            "Public: OPTIONS, DESCRBIE, SETUP, TEARDOWN, PLAY\r\n"
            "\r\n", mCSeq);

    if(sendMessage(mBuffer, strlen(mBuffer)) < 0)
        return false;

    return true;
}

bool RtspConnection::handleCmdDescribe()
{
    MediaSession* session = mRtspServer->loopupMediaSession(mSuffix);

    if(!session)
    {
        LOG_DEBUG("Can't loop up %s session\n", mSuffix.c_str());
        return false;
    }

    mSession = session;
    std::string sdp = session->generateSDPDescription();

    memset((void*)mBuffer, 0, sizeof(mBuffer));
    snprintf((char*)mBuffer, sizeof(mBuffer), 
            "RTSP/1.0 200 OK\r\n"
            "CSeq: %u\r\n"
            "Content-Length: %u\r\n"
            "Content-Type: application/sdp\r\n\r\n"
            "%s", mCSeq, (unsigned int)sdp.size(), sdp.c_str());

    if(sendMessage(mBuffer, strlen(mBuffer)) < 0)
        return false;
    return true;
}

bool RtspConnection::handleCmdSetup()
{
    char sessionName[100];
    if(sscanf(mSuffix.c_str(), "%[^/]/", sessionName) != 1)
        return false;

    MediaSession* session = mRtspServer->loopupMediaSession(sessionName);
    if(!session)
    {
        LOG_DEBUG("Can't loop up %s session\n", sessionName);
        return false;
    }

    if(mTrackId >= MEDIA_MAX_TRACK_NUM || mRtpInstances[mTrackId] || mRtcpInstances[mTrackId])
        return false;

    if(session->isStartMulticast())
    {
        snprintf((char*)mBuffer, sizeof(mBuffer),
                "RTSP/1.0 200 OK\r\n"
                "CSeq: %d\r\n"
                "Transport: RTP/AVP;multicast;"
                "destination=%s;source=%s;port=%d-%d;ttl=255\r\n"
                "Session: %08x\r\n\r\n",
                mCSeq, session->getMulticastDestAddr().c_str(), 
                sockets::getLocalIp().c_str(), 
                session->getMulticastDestRtpPort(mTrackId), session->getMulticastDestRtpPort(mTrackId) + 1, 
                mSessionId);
    }
    else
    {
        if(mIsRtpOverTcp) // rtp over tcp
        {
            createRtpRtcpOverTcp(mTrackId, mSocket.getFd(), mRtpChannel);
            mRtpInstances[mTrackId]->setSessionId(mSessionId);
            session->addRtpInstance(mTrackId, mRtpInstances[mTrackId]);

            snprintf((char*)mBuffer, sizeof(mBuffer), 
                    "RTSP/1.0 200 OK\r\n"
                    "CSeq: %d\r\n"
                    "Transport: RTP/AVP/TCP;unicast;interleaved=%hhu-%hhu\r\n"
                    "Session: %08x\r\n\r\n",
                    mCSeq, mRtpChannel, mRtpChannel + 1, mSessionId);
        }
        else
        {
            if(!createRtpRtcpOverUdp(mTrackId, mPeerIp, mPeerRtpPort, mPeerRtcpPort))
            {
                LOG_WARNING("Failed to create rtp and rtcp\n");
                return false;
            }
            mRtpInstances[mTrackId]->setSessionId(mSessionId);
            mRtcpInstances[mTrackId]->setSessionId(mSessionId);

            session->addRtpInstance(mTrackId, mRtpInstances[mTrackId]);

            snprintf((char*)mBuffer, sizeof(mBuffer), 
                    "RTSP/1.0 200 OK\r\n"
                    "CSeq: %u\r\n"
                    "Transport: RTP/AVP;unicast;client_port=%hu-%hu;server_port=%hu-%hu\r\n"
                    "Session: %08x\r\n\r\n",
                    mCSeq, mPeerRtpPort, mPeerRtcpPort, 
                    mRtpInstances[mTrackId]->getLocalPort(),
                    mRtcpInstances[mTrackId]->getLocalPort(),
                    mSessionId);
        }
    }

    if(sendMessage(mBuffer, strlen(mBuffer)) < 0)
        return false;

    return true;
}

bool RtspConnection::handleCmdPlay()
{
    snprintf((char*)mBuffer, sizeof(mBuffer), 
            "RTSP/1.0 200 OK\r\n"
            "CSeq: %u\r\n"
            "Range: npt=0.000-\r\n"
            "Session: %08x; timeout=60\r\n\r\n",
            mCSeq, mSessionId);

    if(sendMessage(mBuffer, strlen(mBuffer)) < 0)
        return false;
    
    for(int i = 0; i < MEDIA_MAX_TRACK_NUM; ++i)
    {
        if(mRtpInstances[i])
            mRtpInstances[i]->setAlive(true);

        if(mRtcpInstances[i])
            mRtcpInstances[i]->setAlive(true);
    }
    return true;
}

bool RtspConnection::handleCmdTeardown()
{
    snprintf((char*)mBuffer, sizeof(mBuffer), 
            "RTSP/1.0 200 OK\r\n"
            "CSeq: %d\r\n\r\n", mCSeq);
    
    if(sendMessage(mBuffer, strlen(mBuffer)) < 0)
        return false;

    return true;
}

bool RtspConnection::handleCmdGetParameter()
{}

int RtspConnection::sendMessage(void* buf, int size)
{
    mOutputBuffer.append(buf, size);
    int ret = mOutputBuffer.write(mSocket.getFd());
    mOutputBuffer.retrieveAll();

    return ret;
}

int RtspConnection::sendMessage()
{
    int ret = mOutputBuffer.write(mSocket.getFd());
    mOutputBuffer.retrieveAll();
    return ret;
}

bool RtspConnection::createRtpRtcpOverUdp(MediaSession::TrackId trackId, std::string peerIp, uint16_t peerRtpPort, uint16_t peerRtcpPort)
{
    int rtpSockFd, rtcpSockFd;
    uint16_t rtpPort, rtcpPort;

    if(mRtpInstances[trackId] || mRtcpInstances[trackId])
        return false;
    
    int i = 0;
    for(; i < 10; ++i)
    {
        if((rtpSockFd = sockets::createUdpSocket()) < 0)
            return false;
        if((rtcpSockFd = sockets::createUdpSocket()) < 0)
        {
            close(rtpSockFd);
            return false;
        }

        uint16_t port = rand() & 0xfffe;
        if(port < 10000)
            port += 10000;

        rtpPort = port;
        rtcpPort = port + 1;

        int ret = sockets::bind(rtpSockFd, "0.0.0.0", rtpPort);
        if(!ret)
        {
            sockets::close(rtpSockFd);
            sockets::close(rtcpSockFd);
            continue;
        }

        ret = sockets::bind(rtcpSockFd, "0.0.0.0", rtcpPort);
        if(!ret)
        {
            sockets::close(rtpSockFd);
            sockets::close(rtcpSockFd);
            continue;
        }

        break;   
    }

    if(i == 10)
        return false;

    mRtpInstances[trackId] = RtpInstance::createNewOverUdp(rtpSockFd, rtpPort, peerIp, peerRtpPort);
    mRtcpInstances[trackId] = RtcpInstance::createNew(rtcpSockFd, rtcpPort, peerIp, peerRtcpPort);

    return true;
}

bool RtspConnection::createRtpRtcpOverTcp(MediaSession::TrackId trackId, int sockFd, uint8_t rtpChannel)
{
    mRtpInstances[trackId] = RtpInstance::createNewOverTcp(sockFd, rtpChannel);
    return true;
}

void RtspConnection::handleRtpOverTcp()
{
    uint8_t* buf = (uint8_t*)mInputBuffer.peek();
    uint16_t size = (buf[2] << 8) | buf[3];
    if(mInputBuffer.readableBytes() < size + 4)
        return ;

    mInputBuffer.retrieve(size + 4);
}
