#include <iostream>

#include "src/Logging.h"
#include "src/UsageEnvironment.h"
#include "src/base/ThreadPool.h"
#include "src/EventScheduler.h"
#include "src/Event.h"
#include "src/RtspServer.h"
#include "src/MediaSession.h"
#include "src/InetAddress.h"
#include "src/H264FileMediaSource.h"
#include "src/H264RtpSink.h"

int main(int argc, char* argv[])
{
    // if(argc != 2)
    // {
    //     std::cout << "Usage: " << argv[0] << " <h264 file> " << std::endl;
    //     return -1;
    // }

    Logger::setLogLevel(Logger::LOGWARNING);

    EventScheduler* scheduler = EventScheduler::createNew(EventScheduler::POLLER_SELECT);
    ThreadPool* threadPool = ThreadPool::createNew(2);
    UsageEnvironment* env = UsageEnvironment::createNew(scheduler, threadPool);

    Ipv4Address ipAddr("0.0.0.0", 8554);
    RtspServer* server = RtspServer::createNew(env, ipAddr);
    MediaSession* session = MediaSession::createNew("live");
    MediaSource* mediaSource = H264FileMediaSource::createNew(env, "../test.h264");
    RtpSink* rtpSink = H264RtpSink::createNew(env, mediaSource);

    session->addRtpSink(MediaSession::TrackId0, rtpSink);

    server->addMediaSession(session);
    server->start();

    std::cout << "Play the media using the URL: " << server->getUrl(session) << std::endl;

    env->scheduler()->loop();

    return 0;
}