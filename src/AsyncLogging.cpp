#include "AsyncLogging.h"
#include <assert.h>

AsyncLogging* AsyncLogging::mAsyncLogging = NULL;

AsyncLogging::AsyncLogging(std::string file)
    : mFile(file), mRun(true)
{
    mMutex = Mutex::createNew();
    assert(mMutex);

    mCond = Condition::createNew();
    assert(mCond);

    mFp = fopen(mFile.c_str(), "w");
    assert(mFp >= 0);

    for(int i = 0; i < BUFFER_NUM; ++i)
        mFreeBuffer.push(&mBuffer[i]);
    
    mCurBuffer = mFreeBuffer.front();

    start(NULL);
}

AsyncLogging::~AsyncLogging()
{
    for(int i = 0; i < mFlushBuffer.size(); ++i)
    {
        LogBuffer* buffer = mFlushBuffer.front();
        fwrite(buffer->data(), 1, buffer->length(), mFp);
        mFreeBuffer.pop();
    }

    fwrite(mCurBuffer->data(), 1, mCurBuffer->length(), mFp);

    fflush(mFp);
    fclose(mFp);

    mRun = false;
    mCond->broadcast();

    delete mMutex;
    delete mCond;
}

AsyncLogging* AsyncLogging::instance()
{
    if(!mAsyncLogging)
        mAsyncLogging = new AsyncLogging("");
    return mAsyncLogging;
}

void AsyncLogging::append(const char* logLine, int len)
{
    MutexLockGuard mutexLockGuard(mMutex);
    if(mCurBuffer->avail() > len)
        mCurBuffer->append(logLine, len);
    else
    {
        mFreeBuffer.pop();
        mFlushBuffer.push(mCurBuffer);

        // 当前无缓冲区可用
        while(mFreeBuffer.empty())
        {
            mCond->signal();
            mCond->wait(mMutex);
        }

        mCurBuffer = mFreeBuffer.front();
        mCurBuffer->append(logLine, len);
        mCond->signal();
    }
}

void AsyncLogging::run(void* arg)
{
    while(mRun)
    {
        MutexLockGuard mutexLockGuard(mMutex);
        bool ret = mCond->waitTimeout(mMutex, 3000);

        if(!mRun)
            break;
        
        if(ret)
        {
            bool empty = mFreeBuffer.empty();
            int bufferSize = mFlushBuffer.size();
            for(int i = 0; i < bufferSize; ++i)
            {
                LogBuffer* buffer = mFlushBuffer.front();
                fwrite(buffer->data(), 1, buffer->length(), mFp);
                mFlushBuffer.pop();
                buffer->reset();
                mFreeBuffer.push(buffer);
                fflush(mFp);
            }

            if(empty)
                mCond->signal();
        }
        else
        {
            if(mCurBuffer->length() == 0)
                continue;
            fwrite(mCurBuffer->data(), 1, mBuffer->length(), mFp);
            mCurBuffer->reset();
            fflush(mFp);
        }
    }
}