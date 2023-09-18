#ifndef RTP_H
#define RTP_H
#include <stdint.h>
#include <stdlib.h>

#define RTP_VERSION                 2

#define RTP_PLAYLOAD_TYPE_H264      96
#define RTP_PLAYLOAD_TYPE_ACC       97

#define RTP_HEADER_SIZE             12
#define RTP_MAX_PKT_SIZE            1400

struct RtpHeader{
    uint8_t csrcLen : 4;     // csrc计数器
    uint8_t extension : 1;   // RTP报头结尾是否有拓展  
    uint8_t padding : 1;    // 填充标志，报文尾部填充一个或多个字节，不属于有效载荷
    uint8_t version : 2;    // RTP版本号

    uint8_t playloadType : 7; // 有效载荷类型，说明是音频还是视频
    uint8_t marker : 1;     // 标记位，对于视频表示一帧结束，音频表示会话开始

    uint16_t seq;   // 序列号

    uint32_t timestamp;     // 报文时间戳

    uint32_t ssrc;      // 识别同步信源标志

    uint8_t playload[0];    

};

class RtpPacket{
public:
    RtpPacket() : 
        _mBuffer(new uint8_t[RTP_MAX_PKT_SIZE + RTP_HEADER_SIZE + 100]),
        mBuffer(_mBuffer + 4),          // rtp负载开头
        mRtpHeader((RtpHeader*)mBuffer), 
        mSize(0)
    {}

    ~RtpPacket()
    {
        delete _mBuffer;
    }
    
public:
    uint8_t* _mBuffer;
    uint8_t* mBuffer;
    RtpHeader* const mRtpHeader;
    int mSize;
};
#endif