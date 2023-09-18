#pragma once
#ifndef BITSTREAM_H
#define BITSTREAM_H
#include <string>

class BitStream{
public:
    BitStream(uint8_t* buffer, int size);
    ~BitStream() = default;

    int readOneBit();
    int readNBit(int n);
    int readUE();   // 无符号哥伦布编码
    int readSE();   // 有符号哥伦布编码

private:
    uint8_t* start = nullptr;
    int size = 0;
    uint8_t* cur = nullptr;
    int bitsLeft = 8;
};

#endif