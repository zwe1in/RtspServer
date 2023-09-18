#include "Nalu.h"

Nalu::~Nalu()
{
    if(buffer != nullptr)
    {
        delete buffer;
        buffer = nullptr;
    }
}

void Nalu::setBuf(uint8_t* buf, int len_)
{
    if(buffer != nullptr)
    {
        delete buffer;
        buffer = nullptr;
    }
    len = len_;
    buffer = new uint8_t[len];
    memcpy(buffer, buf, len);

}