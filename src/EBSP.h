#pragma once
#ifndef EBSP_H
#define EBSP_H
#include <string>
#include "RBSP.h"

class EBSP{
public:
    EBSP() = default;
    ~EBSP();

    EBSP(const EBSP& other);
    EBSP& operator=(const EBSP& other);

    void getRBSP(RBSP& rbsp);
private:
    uint8_t* buffer = nullptr;
    int len = 0;
};

#endif