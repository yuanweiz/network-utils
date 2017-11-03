#include "BitmapView.h"
#include <assert.h>
const static unsigned char table[]={
    1,2,4,8,16,32,64,128
};
void BitmapView::setZero(size_t pos){
    const unsigned char mask = 0xff;
    ptr_[pos/8] &= ( mask ^ table[pos%8]);
}
void BitmapView::setOne(size_t pos)
{
    ptr_[pos/8] |= table[pos%8];
}
size_t BitmapView::findZeroBit  (uint8_t b){
    b=~b;
    for (size_t i=0;i<8;++i){
        if (table[i] & b)return i;
    }
    //assert(false);
    return 8;
};
