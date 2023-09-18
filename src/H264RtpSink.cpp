#include "H264RtpSink.h"
#include "Logging.h"

H264RtpSink* H264RtpSink::createNew(UsageEnvironment* env, MediaSource* mediaSource)
{
    if(!mediaSource)
        return NULL;
    return new H264RtpSink(env, mediaSource);
}

H264RtpSink::H264RtpSink(UsageEnvironment* env, MediaSource* mediaSource)
    : RtpSink(env, mediaSource, RTP_PLAYLOAD_TYPE_H264), mClockRate(90000), mFps(mediaSource->getFps())
{
    start(1000 / mFps);
}

H264RtpSink::~H264RtpSink(){}

std::string H264RtpSink::getMediaDescription(uint16_t port)
{
    char buf[100] = {0};
    sprintf(buf, "m=video %hu RTP/AVP %d", port, mPlayloadType);

    return buf;
}

std::string H264RtpSink::getAttribute()
{
    char buf[100];
    sprintf(buf, "a=rtpmap:%d H264/%d\r\n", mPlayloadType, mClockRate);
    sprintf(buf + strlen(buf), "a=framerate:%d", mFps);

    return buf;
}

void H264RtpSink::handleFrame(AVFrame* frame)
{
    RtpHeader* rtpHeader = mRtpPacket.mRtpHeader;
    uint8_t naluType = frame->mFrame[0];

    if(frame->mFrameSize <= RTP_MAX_PKT_SIZE)
    {// 直接发
        memcpy(rtpHeader->playload, frame->mFrame, frame->mFrameSize);
        mRtpPacket.mSize = frame->mFrameSize;
        sendRtpPacket(&mRtpPacket);
        mSeq++;
        if((naluType & 0x1F) == 7 || (naluType & 0x1F) == 8)    // sps、pps不用加时间戳
            return ;
    }
    else
    {
        int pktNum = frame->mFrameSize / RTP_MAX_PKT_SIZE;
        int remainPktSize = frame->mFrameSize % RTP_MAX_PKT_SIZE;

        int pos = 1;
        for(int i = 0; i < pktNum; ++i)
        {
            /**
             *    FU Indicator
             *    0 1 2 3 4 5 6 7
             *   +-+-+-+-+-+-+-+-+
             *   |F|NRI|  Type   |
             *   +-+-+-+-+-+-+-+-+
            */
            rtpHeader->playload[0] = (naluType & 0xE0) | 28; // 前三位跟nalu第一个字节一样，28表示为分片

            /*
            *      FU Header
            *    0 1 2 3 4 5 6 7
            *   +-+-+-+-+-+-+-+-+
            *   |S|E|R|  Type   |
            *   +---------------+
            *   S: 第一个包; E: 最后一个包; R: 必须置0，接收忽略该位;
            * */
            rtpHeader->playload[1] = naluType & 0x1F;
            if(i == 0)
                rtpHeader->playload[1] |= 0x80;
            else if(remainPktSize == 0 && i == pktNum - 1)
                rtpHeader->playload[1] |= 0x40;

            memcpy(rtpHeader->playload + 2, frame->mFrame + pos, RTP_MAX_PKT_SIZE);
            mRtpPacket.mSize = RTP_MAX_PKT_SIZE + 2;
            sendRtpPacket(&mRtpPacket);

            mSeq++;
            pos += RTP_MAX_PKT_SIZE;
        }

        /* 多余部分数据 */
        if(remainPktSize > 0)
        {
            rtpHeader->playload[0] = (naluType & 0xE0) | 28 ;
            rtpHeader->playload[1] = naluType & 0x1F;
            rtpHeader->playload[1] |= 0x40;

            memcpy(rtpHeader->playload + 2, frame->mFrame+pos, remainPktSize);
            mRtpPacket.mSize = remainPktSize + 2;
            sendRtpPacket(&mRtpPacket);

            mSeq++;
        }
    }
    mTimestamp += mClockRate / mFps;
}