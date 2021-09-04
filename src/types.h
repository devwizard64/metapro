#ifndef __TYPES_H__
#define __TYPES_H__

#define false   0
#define true    1

#ifdef WIN32
#define SEP     "\\"
#else
#define SEP     "/"
#endif

#ifdef __3DS__
#define FMT_d   "ld"
#define FMT_x   "lx"
#define FMT_X   "lX"
#else
#define FMT_d   "d"
#define FMT_x   "x"
#define FMT_X   "X"
#endif

#ifndef __ASSEMBLER__

#include <stddef.h>
#ifdef __NATIVE__
#include <stdint.h>
typedef  int8_t  s8;
typedef uint8_t  u8;
typedef  int16_t s16;
typedef uint16_t u16;
typedef  int32_t s32;
typedef uint32_t u32;
typedef  int64_t s64;
typedef uint64_t u64;
typedef u8       bool;
#endif
#ifdef __3DS__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#include <3ds.h>
#pragma GCC diagnostic pop
#endif
#ifdef GEKKO
#include <gccore.h>
#endif

typedef unsigned int    uint;
typedef float           f32;
typedef double          f64;

#define lenof(x)        (sizeof((x)) / sizeof((x)[0]))

#define NULLPTR 0
typedef u32 PTR;

#ifdef __GNUC__
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#define __EB__
#endif
#define unused          __attribute__((unused))
#define fallthrough     __attribute__((fallthrough))
#define forceinline     __attribute__((always_inline))
#else
#define unused
#define fallthrough
#define forceinline
#endif

#ifdef __NATIVE__
#define esleep(x)
#endif
#ifdef __3DS__
#define esleep(x)   svcSleepThread(1000000000LL*(x))
#endif
#ifdef GEKKO
#define esleep(x)   {int _i; for (_i = 0; _i < 60*(x); _i++) VIDEO_WaitVSync();}
#endif
#define eexit()     {esleep(3); exit(EXIT_FAILURE);}

#define wprint(...) fprintf(stderr, "warning: " __VA_ARGS__)
#define eprint(...) {fprintf(stderr, "error: " __VA_ARGS__); eexit();}
#ifdef __DEBUG__
#define pdebug printf
#define wdebug wprint
#define edebug eprint
#else
#define pdebug(...) {}
#define wdebug(...) {}
#define edebug(...) {}
#endif

#endif /* __ASSEMBLER__ */

#endif /* __TYPES_H__ */
