#ifndef __BUFFER_H
#define __BUFFER_H
#include <sys/types.h>
#include <vector>
#include <stdio.h>
#include "StreamReader.h"
#include <memory.h>
class StringView {
public:
    StringView( char* c, size_t sz)
        :data_(c),size_(sz)
    {
    }
    size_t size(){return size_;}
    char * peek(){return data_;}
private:
    char * const data_;
    size_t size_;
};

class OnStackBuffer : StreamReaderMixin<OnStackBuffer> {
public:
	OnStackBuffer(); 
	void append(const char *,size_t);
	void append(char);
	void retrieve(size_t);
	void retrieveString(void *, size_t);
	char* data();
	void add(size_t);
	char* peek();
	size_t readable();
    size_t readFile(FILE*);
private:
	size_t start_, end_;
	char data_[1024];
};


class Buffer : StreamReaderMixin<Buffer>{
public:
    Buffer();
	void append(const void *,size_t);
	void append(char);
	void retrieve(size_t);
	void add(size_t);
	void* peek();
	size_t readable();
    size_t readFile(FILE*);
    size_t readFd(int);
private:
    size_t appendable();
    void expand();
    void* end();
    std::vector<char> data_;
    int start_, end_;
    const static int kInitSize = 1024;
};
#endif// __BUFFER_H
