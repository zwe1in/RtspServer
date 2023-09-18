#ifndef H264FILE_MEDIA_SOURCE_H
#define H264FILE_MEDIA_SOURCE_H
#include <string>

#include "UsageEnvironment.h"
#include "MediaSource.h"
#include "base/ThreadPool.h"

class H264FileMediaSource : public MediaSource
{
public:
    static H264FileMediaSource* createNew(UsageEnvironment* env, std::string file);

    H264FileMediaSource(UsageEnvironment* env, std::string file);
    ~H264FileMediaSource();

protected:
    virtual void readFrame();

private:
    int getFrameFromH264File(int fd, uint8_t* frame, int size);

private:
    std::string mFile;
    int mFd;
};


#endif