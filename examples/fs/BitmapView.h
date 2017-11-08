#ifndef __BITMAP_VIEW_H
#define __BITMAP_VIEW_H
#include <sys/types.h>
#include <inttypes.h>
class BitmapView{
public:
    BitmapView(void*ptr,size_t sz):
        ptr_(static_cast<unsigned char*>(ptr)),size_(sz)
    {
    }
    static size_t findZeroBit  (uint8_t );
    size_t size()const {return size_;}
    unsigned char * data(){return ptr_;}
    int nextZeroBit  (size_t)const;
    void setOne(size_t pos);
    void setZero(size_t pos);
    int nextOneBit(size_t=0 );
private:
    unsigned char* const ptr_;
    const size_t size_;
};
#endif// __BITMAP_VIEW_H
