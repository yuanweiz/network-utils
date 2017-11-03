#ifndef __ARRAYVIEW_H
#define __ARRAYVIEW_H
#include <sys/types.h>
#include <assert.h>
template <class T,size_t S>
class ArrayView{
public:
    explicit ArrayView(T*p):
        ptr_(p){}
    ArrayView( const ArrayView &rhs)=default;
    //ArrayView & operator=(const ArrayView&rhs){
    //    ptr_=rhs.ptr_;
    //    return *this;
    //}
    T& operator[] (size_t idx){
        assert(idx<S);
        return ptr_[idx];
    }
    T* data(){return ptr_;}
    size_t size() {return S;}
private:
    T* const ptr_;
};
#endif// __ARRAYVIEW_H
