#ifndef RTPINSTANCE_H
#define RTPINSTANCE_H
#include <string>
#include <unistd.h>
#include <stdio.h>

#include "InetAddress.h"
#include "SocketsOps.h"
#include "Rtp.h"

class RtpInstance{
public:
    enum RtpType{
        RTP_OVER_UDP,
        RTP_OVER_TCP
    };

    static RtpInstance* createNewOverUdp(int localSockFd, uint16_t localPort, std::string destIp, uint16_t destPort)
    {
        return new RtpInstance(localSockFd, localPort, destIp, destPort);
    }

    static RtpInstance* createNewOverTcp(int clientSockFd, uint8_t rtpChannel)
    {
        return new RtpInstance(clientSockFd, rtpChannel);
    }

    RtpInstance(int localSockFd, uint16_t localPort, const std::string& destIp, uint16_t destPort)
        : mRtpType(RTP_OVER_UDP), mSockFd(localSockFd), mLocalPort(localPort),
          mDestAddr(destIp, destPort), mIsAlive(false), mSessionId(0)
    {}

    RtpInstance(int clientSockFd, uint8_t rtpChannel)
        : mRtpType(RTP_OVER_TCP), mSockFd(clientSockFd), 
          mIsAlive(false), mSessionId(0), mRtpChannel(rtpChannel)
    {}

    ~RtpInstance()
    {
        sockets::close(mSockFd);
    }

    uint16_t getLocalPort() const { return mLocalPort; }
    uint16_t getPeerPort() { return mDestAddr.getPort(); }

    int send(RtpPacket* rtpPacket)
    {
        if(mRtpType == RTP_OVER_UDP)
        {
            return sendOverUdp(rtpPacket->mBuffer, rtpPacket->mSize);
        }
        else
        {
            /*|标识符($)|信道|数据长度(2字节)|数据 */
            uint8_t* rtpPktPtr = rtpPacket->_mBuffer;
            rtpPktPtr[0] = '$';
            rtpPktPtr[1] = (uint8_t)mRtpChannel;
            rtpPktPtr[2] = (uint8_t)((rtpPacket->mSize & 0xFF00) >> 8);
            rtpPktPtr[3] = (uint8_t)(rtpPacket->mSize & 0x00FF);
            return sendOverTcp(rtpPktPtr, rtpPacket->mSize + 4);
        }
    }

    bool alive() const { return mIsAlive; }
    int setAlive(bool alive) { mIsAlive = alive; }
    void setSessionId(uint16_t sessionId) { mSessionId = sessionId; }
    uint16_t getSessionId() const { return mSessionId; }

private:
    int sendOverUdp(void* buf, int size)
    {
        return sockets::sendto(mSockFd, buf, size, mDestAddr.getAddr());
    }


    int sendOverTcp(void *buf, int size)
    {
        return sockets::write(mSockFd, buf, size);
    }
private:
    RtpType mRtpType;
    int mSockFd;
    uint16_t mLocalPort; // udp
    Ipv4Address mDestAddr; // udp
    bool mIsAlive;
    uint16_t mSessionId;
    uint8_t mRtpChannel; // tcp
};

class RtcpInstance{
public:
    static RtcpInstance* createNew(int localSockFd, uint16_t localPort, std::string destIp, uint16_t destPort)
    {
        return new RtcpInstance(localSockFd, localPort, destIp, destPort);
    }

    ~RtcpInstance()
    {
        sockets::close(mLocalSockFd);
    }

    int send(void* buf, int size)
    {
        return sockets::sendto(mLocalSockFd, buf, size, mDestAddr.getAddr());
    }

    int recv(void* buf, int size, Ipv4Address* addr)
    {
        return 0;
    }

    uint16_t getLocalPort() const { return mLocalPort; }
    int alive() const { return mIsAlive; }
    int setAlive(bool alive) { mIsAlive = alive; }
    void setSessionId(uint16_t sessionId) { mSessionId = sessionId; }
    uint16_t getSessionId() const { return mSessionId; }
    
public:
    RtcpInstance(int localSockFd, uint16_t localPort, std::string destIp, uint16_t destPort)
        : mLocalSockFd(localSockFd), mLocalPort(localPort), mDestAddr(destIp, destPort),
        mIsAlive(false), mSessionId(0)
    {}

private:
    int mLocalSockFd;
    uint16_t mLocalPort;
    Ipv4Address mDestAddr;
    bool mIsAlive;
    uint16_t mSessionId;
};

#endif