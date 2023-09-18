#ifndef MEDIA_SOURCE_H
#define MEDIA_SOURCE_H
#include <queue>
#include <stdint.h>

#include "UsageEnvironment.h"
#include "base/Mutex.h"

#define FRAME_MAX_SIZE  (1024*500)
#define DEFAULT_FRAME_NUM  4

class AVFrame{
public:
    AVFrame()
        : mBuffer(new uint8_t[FRAME_MAX_SIZE]), mFrameSize(0)
    {} 

    ~AVFrame()
    {
        delete mBuffer;
    }
public:
    uint8_t *mBuffer;
    uint8_t *mFrame; // 当前位置指针
    int mFrameSize;
};


class MediaSource{
public:
    virtual ~MediaSource();

    AVFrame* getFrame();
    void putFrame(AVFrame* frame);
    int getFps() const { return mFps; }

protected:
    MediaSource(UsageEnvironment* env);
    virtual void readFrame() = 0;
    void setFps(int fps) { mFps = fps; }

private:
    static void taskCallback(void*);

protected:
    UsageEnvironment* mEnv;
    AVFrame mAVFrame[DEFAULT_FRAME_NUM];
    std::queue<AVFrame*> mAVFrameInputQueue;
    std::queue<AVFrame*> mAVFrameOutputQueue;
    Mutex* mMutex;
    ThreadPool::Task mTask;
    int mFps;
};
#endif