#include "EBSP.h"

EBSP::~EBSP()
{
    if(buffer != nullptr)
    {
        delete buffer;
        buffer = nullptr;
    }
}

EBSP::EBSP(const EBSP& other)
{
    len = other.len;
    buffer = new uint8_t[len];
    memcpy(buffer, other.buffer, len);    
}

EBSP& EBSP::operator=(const EBSP& other)
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

void EBSP::getRBSP(RBSP& rbsp)
{
    rbsp.len = len;
    rbsp.buffer = new uint8_t[rbsp.len];

    int rbspIdx = 0;
    for(int i = 0; i < len; ++i)
    {
        if(buffer[i] == 0x03)
        {
            if(i > 2 && buffer[i - 1] == 0x00 && buffer[i - 2] == 0x00)
            {
                if(i < len - 1 && (buffer[i + 1] == 0x00 || buffer[i + 1] == 0x01 || buffer[i + 1] == 0x02 || buffer[i + 1] == 0x03))
                {
                    --rbsp.len;
                    continue;
                }
            }
        }
        rbsp.buffer[rbspIdx++] = buffer[i];
    }
}
