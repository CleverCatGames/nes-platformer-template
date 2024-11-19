#ifndef TYPES_H
#define TYPES_H

typedef unsigned int u16;
typedef signed int s16;
typedef unsigned char u8;
typedef signed char s8;

typedef void (*callbackDef)(void);

typedef union {
    u16 word;
    struct {
        u8 lower;
        u8 upper;
    } byte;
} word;

#ifdef __STDC__

// trick ccls into validating __asm__ and __fastcall__
#define __fastcall__
#define __asm__(...)

typedef void* memcpy_result;
typedef unsigned long size_t;

#else

typedef void memcpy_result;
typedef u16 size_t;

#endif

#endif
