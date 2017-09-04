#ifndef __NONCOPYABLE_H
#define __NONCOPYABLE_H
class Noncopyable{
protected:
    Noncopyable()=default;
private:
    Noncopyable& operator = (const Noncopyable&)=delete;
    Noncopyable(const Noncopyable&)=delete;
    Noncopyable(Noncopyable&&)=default;
};
#endif //__NONCOPYABLE_H
