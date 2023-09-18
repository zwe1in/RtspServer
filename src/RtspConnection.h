#ifndef RTSP_CONNECTION_H
#define RTSP_CONNECTION_H
#include <map>
#include "TcpConnection.h"
#include "RtpInstance.h"
#include "MediaSession.h"
#include "RtspServer.h"

class RtspServer;

class RtspConnection : public TcpConnection{
public:
    enum Method{
        OPTIONS, DESCRIBE, SETUP, PLAY, TEARDOWN, GET_PARAMETER, RTCP, NONE
    };

    static RtspConnection* createNew(RtspServer* rtspServer, int sockFd);

    RtspConnection(RtspServer* rtspServer, int sockFd);
    ~RtspConnection();

protected:
    virtual void handleReadBytes();

private:
    bool parseRequest();
    bool parseRequest1(const char* begin, const char* end);
    bool parseRequest2(const char* begin, const char* end);

    bool parseCSeq(std::string& message);
    bool parseAccept(std::string& message);
    bool parseTransport(std::string& message);
    bool parseMediaTrack();
    bool parseSessionId(std::string& message);

    bool handleCmdOption();
    bool handleCmdDescribe();
    bool handleCmdSetup();
    bool handleCmdPlay();
    bool handleCmdTeardown();
    bool handleCmdGetParameter();

    int sendMessage(void* buf, int size);
    int sendMessage();

    bool createRtpRtcpOverUdp(MediaSession::TrackId trackId, std::string peerIp, uint16_t peerRtpPort, uint16_t peerRtcpPort);
    bool createRtpRtcpOverTcp(MediaSession::TrackId trackId, int sockFd, uint8_t rtpChannel);

    void handleRtpOverTcp();

private:
    RtspServer* mRtspServer;
    std::string mPeerIp;
    Method mMethod;
    std::string mUrl;
    std::string mSuffix;
    uint32_t mCSeq;
    uint16_t mPeerRtpPort;
    uint16_t mPeerRtcpPort;
    MediaSession::TrackId mTrackId;
    RtpInstance* mRtpInstances[MEDIA_MAX_TRACK_NUM];
    RtcpInstance* mRtcpInstances[MEDIA_MAX_TRACK_NUM];
    MediaSession* mSession;

    int mSessionId;
    bool mIsRtpOverTcp;
    uint8_t mRtpChannel;
};


#endif