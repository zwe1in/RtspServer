#include "AnnexBReader.h"


AnnexBReader::AnnexBReader(std::string& file_path)
    : filePath(file_path)
{}

int AnnexBReader::open()
{
    fp = fopen(filePath.c_str(), "rb");
    if(fp == nullptr)
    {
        return -1;
    }
    return 0;
}

int AnnexBReader::close()
{
    if(fp != nullptr)
    {
        fclose(fp);
        fp = nullptr;
    }
    if(buffer != nullptr)
    {
        free(buffer);
        buffer = nullptr;
    }
    return 0;
}

int AnnexBReader::readFile()
{
    int tmpBufferLen = 1024;
    uint8_t* tmpBuffer = new uint8_t[tmpBufferLen];

    int len = fread(tmpBuffer, 1, tmpBufferLen, fp);
    if(len > 0)
    {
        // 将新读取的数据段加到总的buffer末尾
        uint8_t* nbuffer = new uint8_t[len + bufferLen];
        memcpy(nbuffer, buffer, bufferLen);
        memcpy(nbuffer + bufferLen, tmpBuffer, len);
        bufferLen += len;

        if(buffer != nullptr)
            delete buffer;

        buffer = nbuffer;
        nbuffer = nullptr;
    }
    delete tmpBuffer;
    return len;
}

bool AnnexBReader::findStartCode(int& startCodeType, uint8_t* buf, int bufLen)
{
    if(bufLen < 3)
        return false;
    if(buf[0] == 0 && buf[1] == 0 && buf[2] == 1)
    {
        startCodeType = 3;
        return true;
    }

    if(bufLen < 4)
        return false;
    if(buf[0] == 0 && buf[1] == 0 && buf[2] == 0 && buf[3] == 1)
    {
        startCodeType = 4;
        return true;
    }
    return false;
}

int AnnexBReader::readNalu(Nalu& nalu)
{
    while(true)
    {
        if(bufferLen <= 0)
        {
            // 当前buffer没有数据处理，先读取部分数据进来
            int len = readFile();
            if(len <= 0)
                isEnd = true;
        }

        uint8_t* buf = buffer;
        int startCodeType = 0;

        if(!findStartCode(startCodeType, buf, bufferLen))
            break;

        nalu.startCodeType = startCodeType;
        int endPos = -1;
        for(int i = startCodeType; i < bufferLen; ++i)
        {
            int startCode = 0;
            if(findStartCode(startCode, buf + i, bufferLen - i))
            {
                endPos = i;
                break;
            }
        }

        if(endPos > 0)
        {
            nalu.setBuf(buffer, bufferLen);
            
            uint8_t* nbuffer = new uint8_t[bufferLen - endPos];
            memcpy(nbuffer, buffer + endPos, bufferLen - endPos);
            if(buffer != nullptr)
            {
                delete buffer;
                buffer = nullptr;
            }
            buffer = nbuffer;
            bufferLen = bufferLen - endPos;
            nbuffer = nullptr;

            return 0;
        }
        else
        {
            if(isEnd)
            {
                nalu.setBuf(buffer, bufferLen);
                // 到达文件结尾
                if(buffer != nullptr)
                {
                    delete buffer;
                    buffer = nullptr;
                }
                bufferLen = 0;
                return 0;
            }
            else
            {
                // buffer不够，继续往后面加数据，然后重新解析整条
                int len = readFile();
                if(len <= 0)
                    isEnd = true;
            }
        }
    }
    return -1;
}