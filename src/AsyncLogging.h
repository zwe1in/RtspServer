#ifndef ASYNCLOGGING_H
#define ASYNCLOGGING_H
#include <string>
#include <queue>
#include <string.h>

#include "base/Thread.h"
#include "base/Mutex.h"
#include "base/Condition.h"

class LogBuffer{
public:
    LogBuffer() : mCurPtr(mData) { }
    ~LogBuffer() = default;

    const char* data() const { return mData; }
    int length() const { return (int)(mCurPtr - mData); }
    char* current() const { return mCurPtr; }
    int avail() const { return (int)(end() - mCurPtr); }
    void add(int len) { mCurPtr += len; }
    void reset() { mCurPtr = mData; }
    void bzero() { memset(mData, 0, BUFFER_SIZE); }

    void append(const char* buf, size_t len)
    {
        if(avail() > len)
        {
            memcpy(mCurPtr, buf, len);
            mCurPtr += len;
        }
    }

private:
    const char* end() const { return mData + BUFFER_SIZE; }
private:
    static const int BUFFER_SIZE = 1024*1024;
    char mData[BUFFER_SIZE];
    char* mCurPtr;
};


class AsyncLogging : public Thread{
public:
    virtual ~AsyncLogging();
    static AsyncLogging* instance();
    void append(const char* logLine, int len);

protected:
    AsyncLogging(std::string file);
    virtual void run(void* arg);

private:
    static const int BUFFER_NUM = 4;
    
    Mutex* mMutex;
    Condition* mCond;
    std::string mFile;
    FILE* mFp;
    bool mRun;

    LogBuffer mBuffer[BUFFER_NUM];
    LogBuffer* mCurBuffer;
    std::queue<LogBuffer*> mFreeBuffer;
    std::queue<LogBuffer*> mFlushBuffer;

    static AsyncLogging* mAsyncLogging;
};
#endif