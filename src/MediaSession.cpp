#include <time.h>
#include <stdio.h>
#include <string.h>
#include <algorithm>
#include <assert.h>

#include "MediaSession.h"
#include "SocketsOps.h"
#include "Logging.h"


MediaSession* MediaSession::createNew(std::string sessionName)
{
    return new MediaSession(sessionName);
}

MediaSession::MediaSession(const std::string& sessionName)
    : mSessionName(sessionName), mIsStartMulticast(false)
{
    mTracks[0].mTrackId = TrackId0;
    mTracks[1].mTrackId = TrackId1;
    mTracks[0].mIsAlive = false;
    mTracks[1].mIsAlive = false;

    for(int i = 0; i < MEDIA_MAX_TRACK_NUM; ++i)
    {
        mMulticastRtpInstances[i] = NULL;
        mMulticastRtcpInstances[i] = NULL;
    }
}

MediaSession::~MediaSession()
{
    for(int i = 0; i < MEDIA_MAX_TRACK_NUM; ++i)
    {
        if(mMulticastRtpInstances[i])
        {
            this->removeRtpInstance(mMulticastRtpInstances[i]);
            delete mMulticastRtpInstances[i];
        }
        
        if(mMulticastRtcpInstances[i])
            delete mMulticastRtcpInstances[i];
    }
}

std::string MediaSession::generateSDPDescription()
{
    if(!mSdp.empty())
        return mSdp;
    
    std::string ip = sockets::getLocalIp();
    char buf[2048] = { 0 };

    snprintf(buf, sizeof(buf),
        "v=0\r\n"
        "o=- 9%ld 1 IN IP4 %s\r\n"
        "t=0 0\r\n"
        "a=control:*\r\n"
        "a=type:broadcast\r\n",
        (long)time(NULL), ip.c_str());

    if(isStartMulticast())
    {
        snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf),
                "a=rtcp-unicast: reflection\r\n");
    }

    for(int i = 0; i < MEDIA_MAX_TRACK_NUM; ++i)
    {
        uint16_t port = 0;

        if(!mTracks[i].mIsAlive)
            continue;
        
        if(isStartMulticast())
            port = getMulticastDestRtpPort((TrackId)i);
        
        snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf),
                "%s\r\n", mTracks[i].mRtpSink->getMediaDescription(port).c_str());
    
        if(startMulticast())
            snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), 
                    "c=IN IP4 %s/255\r\n", getMulticastDestAddr().c_str());
        else
            snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), 
                    "c=IN IP4 0.0.0.0\r\n");

        snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf),
                "%s\r\n", mTracks[i].mRtpSink->getAttribute().c_str());
        
        snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf),
                "a=control:track%d\r\n", mTracks[i].mTrackId);
    }

    mSdp = buf;
    return mSdp;
}

MediaSession::Track* MediaSession::getTrack(MediaSession::TrackId trackId)
{
    for(int i = 0; i < MEDIA_MAX_TRACK_NUM; ++i)
    {
        if(mTracks[i].mTrackId == trackId)
            return &mTracks[i];
    }
    return NULL;
}

bool MediaSession::addRtpSink(MediaSession::TrackId trackId, RtpSink* rtpSink)
{
    Track* track = getTrack(trackId);

    if(!track)
        return false;

    track->mRtpSink = rtpSink;
    track->mIsAlive = true;

    rtpSink->setSendFrameCallback(sendPacketCallback, this, track);

    return true;
}

bool MediaSession::addRtpInstance(MediaSession::TrackId trackId, RtpInstance* rtpInstance)
{
    Track* track = getTrack(trackId);
    if(!track || !track->mIsAlive)
        return false;
    track->mRtpInstance.push_back(rtpInstance);

    return true;
}

bool MediaSession::removeRtpInstance(RtpInstance* rtpInstance)
{
    for(int i = 0; i < MEDIA_MAX_TRACK_NUM; ++i)
    {
        if(!mTracks[i].mIsAlive)
            continue;
        
        std::list<RtpInstance*>::iterator iter = std::find(mTracks[i].mRtpInstance.begin(), mTracks[i].mRtpInstance.end(), rtpInstance);

        if(iter == mTracks[i].mRtpInstance.end())
            continue;
        
        mTracks[i].mRtpInstance.erase(iter);
        return true;
    }
    return false;
}

void MediaSession::sendPacketCallback(void* arg1, void* arg2, RtpPacket* rtpPacket)
{
    MediaSession* mediaSession = (MediaSession*)arg1;
    MediaSession::Track* track = (MediaSession::Track*)arg2;

    mediaSession->sendPacket(track, rtpPacket);
}

void MediaSession::sendPacket(MediaSession::Track* track, RtpPacket* rtpPacket)
{
    std::list<RtpInstance*>::iterator iter = track->mRtpInstance.begin();
    // 遍历要发送的Rtp远端
    for(; iter != track->mRtpInstance.end(); ++iter)
    {
        if((*iter)->alive())
            (*iter)->send(rtpPacket);
    }
}

bool MediaSession::startMulticast()
{
    /* 生成随机多播地址 */
    struct sockaddr_in addr = { 0 };
    uint32_t range = 0xE8FFFFFF - 0xE8000100;
    addr.sin_addr.s_addr = htonl(0xE8000100 + (rand()) % range);
    mMulticastAddr = inet_ntoa(addr.sin_addr);

    int rtpSockFd1, rtcpSockFd1;
    int rtpSockFd2, rtcpSockFd2;
    uint16_t rtpPort1, rtcpPort1;
    uint16_t rtpPort2, rtcpPort2;

    rtpSockFd1 = sockets::createUdpSocket();
    assert(rtpSockFd1 > 0);

    rtpSockFd2 = sockets::createUdpSocket();
    assert(rtpSockFd2 > 0);

    rtcpSockFd1 = sockets::createUdpSocket();
    assert(rtcpSockFd1 > 0);

    rtcpSockFd2 = sockets::createUdpSocket();
    assert(rtcpSockFd2 > 0);

    uint16_t port = rand() % 0xfffe;
    if(port < 10000)
        port += 10000;
    
    rtpPort1 = port;
    rtcpPort1 = port + 1;
    rtpPort2 = rtcpPort1 + 1;
    rtcpPort2 = rtpPort2 + 1;

    mMulticastRtpInstances[TrackId0] = RtpInstance::createNewOverUdp(rtpSockFd1, 0, mMulticastAddr, rtpPort1);
    mMulticastRtpInstances[TrackId1] = RtpInstance::createNewOverUdp(rtpSockFd2, 0, mMulticastAddr, rtpPort2);
    mMulticastRtcpInstances[TrackId0] = RtcpInstance::createNew(rtcpSockFd1, 0, mMulticastAddr, rtcpPort1);
    mMulticastRtcpInstances[TrackId1] = RtcpInstance::createNew(rtcpSockFd2, 0, mMulticastAddr, rtcpPort2);

    this->addRtpInstance(TrackId0, mMulticastRtpInstances[TrackId0]);
    this->addRtpInstance(TrackId1, mMulticastRtpInstances[TrackId1]);
    mMulticastRtcpInstances[TrackId0]->setAlive(true);
    mMulticastRtcpInstances[TrackId1]->setAlive(true);

    mIsStartMulticast = true;

    return true;
}

bool MediaSession::isStartMulticast()
{
    return mIsStartMulticast;
}

uint16_t MediaSession::getMulticastDestRtpPort(TrackId trackId)
{
    if(trackId > TrackId1 || !mMulticastRtcpInstances[trackId])
        return -1;

    return mMulticastRtpInstances[trackId]->getPeerPort();
}