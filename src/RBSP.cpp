#include "RBSP.h"

RBSP::~RBSP()
{
    if(buffer != nullptr)
    {
        delete buffer;
        buffer = nullptr;
    }
}

RBSP::RBSP(const RBSP& other)
{
    len = other.len;
    buffer = new uint8_t[len];
    memcpy(buffer, other.buffer, len);    
}

RBSP& RBSP::operator=(const RBSP& other)
{
    if(buffer != nullptr)
    {
        delete buffer;
        buffer = nullptr;
    }

    len = other.len;
    buffer = new uint8_t[len];
    memcpy(buffer, other.buffer, len); 
    return *this;
}