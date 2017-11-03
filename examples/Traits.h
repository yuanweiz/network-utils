#ifndef __TRAITS_H
#define __TRAITS_H
#include <sys/types.h>
enum class UnitType{Scalar,Block,BlockGroup, Inode};
template<UnitType U1, UnitType U2, size_t F>
struct UnitTrans{
    size_t value()const{return F;}
};

template<class T>
struct EnableOps{
    bool operator < (const T&rhs)const{
        return self().value()<rhs.value();
    }
    bool operator == (const T& rhs)const{
        return self().value()==rhs.value();
    }
    bool operator > (const T&rhs)const{
        return self().value()>rhs.value();
    }
    bool operator >= (const T& rhs)const{
        return !( *this < rhs);
    }
    bool operator <= (const T& rhs)const{
        return !( *this > rhs);
    }
    bool operator!=(const T& rhs)const{
        return !(*this == rhs);
    }
    const T& self()const{
        return * static_cast<const T*>(this);
    }
};
template <UnitType U>
struct Unit : EnableOps<Unit<U>>{
public:
    //explicit 
        Unit(size_t v):
        v_(v){}
    //explicit operator size_t() {return v_;}
    //is operator-() useful?
    Unit operator + (const Unit& rhs)const{
        return Unit(v_+rhs.v_);
    } 

    Unit operator + (size_t v)const{
        return Unit(v_+v);
    } 
    Unit operator++ (int){
        return Unit(v_++);
    }
    Unit& operator++ (){
        ++v_;
        return *this;
    }
    template <UnitType U2,size_t F>
    Unit<U2> operator * ( UnitTrans<U,U2,F> )const{
        return Unit<U2>(v_*F);
    }

    template <UnitType U2,size_t F>
    Unit<U2> operator / ( UnitTrans<U2,U,F> )const{
        return Unit<U2>(v_/F);
    }

    template <UnitType U2,size_t F>
    Unit operator % ( UnitTrans<U2,U,F> )const{
        return Unit(v_%F);
    }

    size_t operator % ( const Unit& rhs )const{
        return v_%rhs.v_;
    }
    
    size_t value() const{return v_;}
private:
    size_t v_;
};

//specialized
template <> struct Unit<UnitType::Scalar>
{
    Unit(size_t v):v_(v){}
    const static UnitType U=UnitType::Scalar;
    template <UnitType U2,size_t F>
    Unit<U2> operator * ( UnitTrans<U,U2,F> )const{
        return Unit<U2>(v_*F);
    }

    template <UnitType U2,size_t F>
    Unit<U2> operator / ( UnitTrans<U2,U,F> )const{
        return Unit<U2>(v_/F);
    }
    operator size_t () const {return v_;}
    Unit& operator=(size_t v){
        v_=v;return *this;
    }
private:
    size_t v_;
};

#endif
