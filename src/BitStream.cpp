#include "BitStream.h"

BitStream::BitStream(uint8_t* buffer, int size_)
{
    start = buffer;
    cur = buffer;
    size = size_;
}

int BitStream::readOneBit()
{
    int r = 0;
    --bitsLeft;
    r = ((*cur) >> bitsLeft) & 0x01;
    if(bitsLeft == 0)
    {
        ++cur;
        bitsLeft = 8;
    }
}

int BitStream::readNBit(int n)
{
    int r = 0;
    for(int i = 0; i < n; ++i)
        r |= (readOneBit() << (n - i - 1));
    return r;
}

int BitStream::readUE()
{
    int r = 0, i = 0;
    while((readOneBit() == 0) && (i < 32))
        ++i;
    // 真正长度为i + 1，但是已经读了内容的第一位
    r = readNBit(i);
    r |= (1 << i) - 1;  // 编码时，数据部分是加了1
    return r;
}

int BitStream::readSE()
{
    int r = readUE();
    if(r & 0x01)
        r = (r + 1) / 2;    // 去掉最后一位, 最后一位为1，是正数
    else
        r = -(r / 2);   // 去掉最后一位，最后一位为0，是负数
    return r;
}