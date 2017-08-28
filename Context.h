#ifndef __CONTEXT_H
#define __CONTEXT_H
#include <stdint.h>
union Context_t{
    uint64_t u64[2];
    int64_t i64[2];
    uint32_t u32[4];
    int32_t i32[4];
    uint8_t u8[16];
    int8_t i8[16];
};
struct EnableContext{
    Context_t ctx;
};
#endif// __CONTEXT_H
