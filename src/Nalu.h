#ifndef NALU_H
#define NALU_H
#include <string>
#include <string.h>

struct Nalu{
    uint8_t* buffer = nullptr;
    int len = 0;
    int startCodeType = 0;

    Nalu() = default;
    ~Nalu();
    void setBuf(uint8_t* buf, int len_);
};

#endif