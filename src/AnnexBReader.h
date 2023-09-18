#pragma once
#ifndef ANNEXBREADER_H
#define ANNEXBREADER_H
#include <string>
#include <string.h>
#include "Nalu.h"

class AnnexBReader{
public:
    AnnexBReader(std::string& file_path);
    ~AnnexBReader() = default;

    int open();
    int close();
    int readNalu(Nalu& nalu);

private:
    int readFile();
    bool findStartCode(int& startCodeType, uint8_t* buf, int bufLen);
private:
    FILE* fp = nullptr;
    std::string filePath;

    bool isEnd = false;
    uint8_t* buffer = nullptr;
    int bufferLen = 0;
};

#endif