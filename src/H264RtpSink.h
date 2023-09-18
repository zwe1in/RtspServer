#ifndef H264_RTP_SINK_H
#define H264_RTP_SINK_H
#include "RtpSink.h"

class H264RtpSink : public RtpSink
{
public:
    static H264RtpSink* createNew(UsageEnvironment* env, MediaSource* mediaSource);

    H264RtpSink(UsageEnvironment* env, MediaSource* MediaSource);
    virtual ~H264RtpSink();

    virtual std::string getMediaDescription(uint16_t port);
    virtual std::string getAttribute();
    virtual void handleFrame(AVFrame* frame);

private:
    RtpPacket mRtpPacket;
    int mClockRate;
    int mFps;
};

#endif