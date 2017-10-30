#include <wz/Exception.h>
#include <wz/Logging.h>
#include <stdio.h>
#include <execinfo.h>
void bar(){
    std::string str;
    LOG_DEBUG << "before backTrace";
    ::backTrace(str);
    //void* arr[20];
    //::backtrace(arr,20);
    //LOG_DEBUG << __builtin_frame_address (1);
    LOG_DEBUG << "after backTrace";
    puts(str.c_str());
}
void foo(){
    puts("bar() first time");
    bar();
    puts("bar() second time");
    bar();
}


int main(){
    foo();
}
