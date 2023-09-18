#ifndef MEDIASESSION_H
#define MEDIASESSION_H
#include <string>
#include <list>

#include "RtpInstance.h"
#include "RtpSink.h"

#define MEDIA_MAX_TRACK_NUM  2

class MediaSession
{
public:
    enum TrackId
    {
        TrackIdNone = -1,
        TrackId0 = 0,
        TrackId1 = 1
    };

    static MediaSession* createNew(std::string sessionName);

    MediaSession(const std::string& sessionName);
    ~MediaSession();

    std::string getSessionName() const { return mSessionName; }
    std::string generateSDPDescription();
    bool addRtpSink(MediaSession::TrackId trackId, RtpSink* rtpSink);
    bool addRtpInstance(MediaSession::TrackId trackId, RtpInstance* rtpInstance);
    bool removeRtpInstance(RtpInstance* rtpInstance);
    bool startMulticast();
    bool isStartMulticast();
    std::string getMulticastDestAddr() const { return mMulticastAddr; }
    uint16_t getMulticastDestRtpPort(TrackId trackId);

private:
    class Track
    {
    public:
        RtpSink* mRtpSink;
        int mTrackId;
        bool mIsAlive;
        std::list<RtpInstance*> mRtpInstance;
    };

    Track* getTrack(MediaSession::TrackId trackId);
    static void sendPacketCallback(void* arg1, void* arg2, RtpPacket* rtpPacket);
    void sendPacket(MediaSession::Track* track, RtpPacket* rtpPacket);

private:
    std::string mSessionName;
    std::string mSdp;
    Track mTracks[MEDIA_MAX_TRACK_NUM];
    bool mIsStartMulticast;
    std::string mMulticastAddr;
    RtpInstance* mMulticastRtpInstances[MEDIA_MAX_TRACK_NUM];
    RtcpInstance* mMulticastRtcpInstances[MEDIA_MAX_TRACK_NUM];
};

#endif