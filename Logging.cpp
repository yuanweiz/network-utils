#include "Logging.h"

#include <string.h>
#include <algorithm>
#include <type_traits>
#include <unistd.h>

const char* logLevelStr[]{
    "[ERROR ]",
	"[TRACE ]",
	"[INFO  ]",
	"[DEBUG ]",
	"[VERB  ]",
};

void* LogStream::data(){
    return buffer_.peek();
}

size_t LogStream::size(){
    return buffer_.readable();
}
static char digit[] = "0123456789";
template <class T>
void LogStream::printIntegral(T v){
    static_assert(std::is_integral<T>::value,"Expecting integral type");
    T i = v;
    //if (i<0) {
    //    buffer_.append('-');
    //    i=-i;
    //    printIntegral<T>(i);
    //}
    char buf[32], *p=buf;
    size_t len=0;
    while (i!=0){
        ++len;
        *p++=digit[i%10];
        i/=10;
    }
    if (v <0){
        ++len;
        *p='-';
    }
    else if (v==0){
        ++len;
        *p='0';
    }
    std::reverse(buf,buf+len);
    buffer_.append(buf,len);
}

LogStream & LogStream::operator<< (float f){
    return this->operator<<(static_cast<double>(f));
}

LogStream & LogStream::operator<< (double f){
    char buf[32];
    int len =::snprintf(buf,32,"%f",f);
    buffer_.append(buf,len);
    return *this;
}

LogStream & LogStream::operator<< (const void * p){
    char buf[32];
    int len =::snprintf(buf,32,"%p",p);
    buffer_.append(buf,len);
    return *this;
}

LogStream & LogStream::operator << (int64_t i)
{
    printIntegral(i);
    return *this;
}
LogStream & LogStream::operator << (uint64_t i)
{
    printIntegral(i);
    return *this;
}
LogStream & LogStream::operator << (int32_t i)
{
    printIntegral(i);
    return *this;
}
LogStream & LogStream::operator << (uint32_t i)
{
    printIntegral(i);
    return *this;
}
LogStream & LogStream::operator << (int16_t i)
{
    printIntegral(i);
    return *this;
}
LogStream & LogStream::operator << (uint16_t i)
{
    printIntegral(i);
    return *this;
}

LogStream & LogStream::operator << (const char* str) {
    buffer_.append(str,strlen(str));
    return *this;
}
LogStream & LogStream::operator << (char ch) 
{
    buffer_.append(ch);
    return *this;
}

LogStream & LogStream::operator << (const std::string & s) 
{
    buffer_.append(s.c_str(),s.size());
    return *this;
}

//Logger
Logger::Logger(const char * file, int lineno, int logLevel)
{
	stream() << logLevelStr[logLevel] << 
#ifdef _WIN32
        ::strrchr(file,'\\')+1 
#else 
        __builtin_strrchr(file, '/')+1
#endif
        << ":" << lineno << ":" ;
}
Logger::~Logger(){
    stream() << "\n";
    //auto n=printf("%s",static_cast<char*>(stream().data()));
    ::write(1,stream().data(),stream().size());
    //char fmt[32];
    //snprintf(fmt,sizeof(fmt),"%%%lds", stream().size());
    //printf(fmt,stream().data());
    //::snprintf(buffer_.peek(), buffer_.readable(),"%s");
}
int Logger::level = Logger::LogVerbose;

