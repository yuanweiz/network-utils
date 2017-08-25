#ifndef __LOGGING_H
#define __LOGGING_H
#include <string>
#include <stdint.h>
#include "Buffer.h"

class LogStream {
public:
    LogStream & operator << (float);
    LogStream & operator << (double);
    LogStream & operator << (int64_t);
    LogStream & operator << (uint64_t);
    LogStream & operator << (int32_t);
    LogStream & operator << (uint32_t);
    LogStream & operator << (int16_t);
    LogStream & operator << (uint16_t);
	LogStream & operator << (const char* str) ;
	LogStream & operator << (const void* str) ;
	LogStream & operator << (char ch) ;
	LogStream & operator << (const std::string & s) ;
    void * data();
    size_t size();
private:
    template < class T> void printIntegral(T);
    OnStackBuffer buffer_;
};

class Logger{
public:
	Logger(const char * file, int lineno, int logLevel);
	static void setLevel(int l) { level = l; }
	static int getLevel() { return level; }
	enum {LogError,LogTrace,LogInfo, LogDebug, LogVerbose};
	static int level;
    ~Logger();
    LogStream& stream(){return stream_;}
private:
    LogStream stream_;
};

#define LOG_ERROR Logger(__FILE__, __LINE__, Logger::LogError).stream()
#define LOG_DEBUG if (Logger::getLevel() >= Logger::LogDebug)\
	Logger(__FILE__, __LINE__, Logger::LogDebug).stream()
#define LOG_INFO if (Logger::getLevel() >= Logger::LogInfo)\
	Logger(__FILE__, __LINE__, Logger::LogInfo).stream()
#define LOG_VERBOSE if (Logger::getLevel() >= Logger::LogVerbose)\
	Logger(__FILE__, __LINE__, Logger::LogVerbose).stream()
#define LOG_TRACE if (Logger::getLevel() >= Logger::LogTrace)\
	Logger(__FILE__, __LINE__, Logger::LogTrace).stream()

#endif// __LOGGING_H
