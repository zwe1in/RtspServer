#include "MediaSource.h"
#include "Logging.h"

MediaSource::MediaSource(UsageEnvironment* env)
    : mEnv(env)
{
    mMutex = Mutex::createNew();
    for(int i = 0; i < DEFAULT_FRAME_NUM; ++i)
        mAVFrameInputQueue.push(&mAVFrame[i]);

    mTask.setTaskCallback(taskCallback, this);
}

MediaSource::~MediaSource()
{
    delete mMutex;
}

AVFrame* MediaSource::getFrame()
{
    MutexLockGuard mutexLockGuard(mMutex);
    if(mAVFrameOutputQueue.empty())
        return NULL;
    
    AVFrame* frame = mAVFrameOutputQueue.front();
    mAVFrameOutputQueue.pop();

    return frame;
}

void MediaSource::putFrame(AVFrame* frame)
{
    MutexLockGuard mutexLockGuard(mMutex);
    mAVFrameInputQueue.push(frame);
    mEnv->threadPool()->addTask(mTask);
}

void MediaSource::taskCallback(void* arg)
{
    MediaSource* source = (MediaSource*)arg;
    source->readFrame();
}
