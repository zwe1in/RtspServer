#ifndef LOGGING_H
#define LOGGING_H
#include <string>
#include <string.h>

class Logger{
public:
    enum LogLevel{
        LOGINFO,
        LOGERROR,
        LOGWARNING,
        LOGDEBUG
    };

    Logger();
    ~Logger();

    static void setLogFile(std::string file);
    static std::string getLogFile();
    static void setLogLevel(LogLevel level);
    static LogLevel getLogLevel();

    void write(LogLevel level, const char* file, 
                const char* func, int line, const char* format, ...);

private:
    char mData[4096];
    char* mCurPtr;
    LogLevel mThisLogLevel;

    static LogLevel mLogLevel;
    static std::string mLogFile;
    static bool mIsStdout;
};

#define LOG_INFO(format, ...) \
    if(Logger::LOGINFO <= Logger::getLogLevel()) \
        Logger().write(Logger::LOGINFO, __FILE__, __FUNCTION__, __LINE__, format, ##__VA_ARGS__)

#define LOG_ERROR(format, ...) \
    if(Logger::LOGERROR <= Logger::getLogLevel()) \
        Logger().write(Logger::LOGERROR, __FILE__, __FUNCTION__, __LINE__, format, ##__VA_ARGS__)

#define LOG_WARNING(format, ...) \
    if(Logger::LOGWARNING <= Logger::getLogLevel()) \
        Logger().write(Logger::LOGWARNING, __FILE__, __FUNCTION__, __LINE__, format, ##__VA_ARGS__)

#define LOG_DEBUG(format, ...) \
    if(Logger::LOGDEBUG <= Logger::getLogLevel()) \
        Logger().write(Logger::LOGDEBUG, __FILE__, __FUNCTION__, __LINE__, format, ##__VA_ARGS__)
#endif