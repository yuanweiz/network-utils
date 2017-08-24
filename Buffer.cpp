#include <string.h>
#include <assert.h>
#include "Buffer.h"
OnStackBuffer::OnStackBuffer() :start_(0), end_(0) {
}
char * OnStackBuffer::data() {
	return data_;
}

char * OnStackBuffer::peek() {
	return data_ + start_;
}
void OnStackBuffer::retrieve(size_t sz) {
	start_ += static_cast<int>(sz);
}
size_t OnStackBuffer::readable() {
	assert(end_ >= start_);
	return end_ - start_;
}

void OnStackBuffer::retrieveString(void *dst, size_t sz) {
	::memcpy(dst, peek(), sz);
	retrieve(sz);
}

void OnStackBuffer::append(const char*src, size_t sz) {
	::memcpy(data_+end_, src, sz);
	end_ += static_cast<int>(sz);
}
void OnStackBuffer::append(char c){
    data_[end_++] = c;
}
size_t OnStackBuffer::readFile(FILE* fp){
    auto avail = readable();
    ::memmove(data_, peek(), avail);
    start_ = 0;
    end_ = avail;
    auto sz= ::fread(data_ + end_, 1,sizeof(data_) - end_,fp);
	end_ += static_cast<int>(sz);
    return sz;
}


// adaptive buffer on heap
Buffer::Buffer(): data_(kInitSize),start_(0), end_(0){
}

size_t Buffer::readable(){
    return end_-start_;
}
void* Buffer::end(){
    return data_.data()+ end_;
}
void * Buffer::peek(){
    return data_.data()+start_;
}

void Buffer::expand(){
    data_.resize(data_.size()*2);
}

size_t Buffer::appendable(){
    return data_.size()-end_;
}
void Buffer::append(const void * ptr,size_t sz){
    while (appendable() < sz){
        expand();
    }
    ::memcpy(end(), ptr, sz);
	end_ += static_cast<int>(sz);
}

void Buffer::retrieve(size_t sz){
    start_+=static_cast<int>(sz);
}

void Buffer::append(char c){
    *static_cast<char*>(end())=c;
    end_++;
}
