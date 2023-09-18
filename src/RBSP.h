#pragma once
#ifndef RBSP_H
#define RBSP_H
#include <string>

class RBSP{
public:
    RBSP() = default;
    ~RBSP();

    RBSP(const RBSP& other);
    RBSP& operator=(const RBSP& other);
public:
    uint8_t* buffer = nullptr;
    int len = 0;
};

#endif