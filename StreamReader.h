#ifndef __STREAMREADER_H
#define __STREAMREADER_H

#include <stdint.h>
#include <string.h>

//StreamReaderMixin:
//require a base class with following 
// methods:
//
// void retrieve(size_t);
// void * peek();
//

template < class Buffer>
class StreamReaderMixin {
public:
    int8_t readInt8() {
        int8_t i = peekInt8();
        buffer().retrieve(sizeof(int8_t));
        return i;
    }

    int8_t peekInt8() {
        return peekInt<int8_t>();
    }

    int16_t readInt16() {
        int16_t i = peekInt16();
        buffer().retrieve(sizeof(int16_t));
        return i;
    }

    int16_t peekInt16() {
        return peekInt<int16_t>();
    }

    int32_t readInt32() {
        int32_t i = peekInt32();
        buffer().retrieve(sizeof(int32_t));
        return i;
    }

    int32_t peekInt32() {
        return peekInt<int32_t>();
    }

protected:
    StreamReaderMixin(){}
    template <class T> T peekInt(){
        T t;
        ::memcpy(&t, buffer().peek(), sizeof(T));
        return t;
    }
    Buffer& buffer(){
        return *static_cast<Buffer*>(this);
    }
};

#endif// __STREAMREADER_H
