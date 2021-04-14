#ifndef _TYPES_H_
#define _TYPES_H_

#ifndef __ASSEMBLER__

#include <stddef.h>

#ifdef _NATIVE
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
#ifdef _3DS
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

#endif /* __ASSEMBLER__ */

#define false   0
#define true    1

#ifdef __GNUC__
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#define _EB
#endif
#define unused          __attribute__((unused))
#define fallthrough     __attribute__((fallthrough))
#define forceinline     __attribute__((always_inline))
#else
#define unused
#define fallthrough
#define forceinline
#endif

#ifdef WIN32
#define SEP     "\\"
#else
#define SEP     "/"
#endif

#ifdef _3DS
#define FMT_d   "ld"
#define FMT_x   "lx"
#define FMT_X   "lX"
#else
#define FMT_d   "d"
#define FMT_x   "x"
#define FMT_X   "X"
#endif

#ifdef _3DS
#define eexit() {svcSleepThread(3000000000LL); exit(EXIT_FAILURE);}
#else
#define eexit() exit(EXIT_FAILURE)
#endif

#define wprint(...) fprintf(stderr, "warning: " __VA_ARGS__)
#define eprint(...) {fprintf(stderr, "error: " __VA_ARGS__); eexit();}
#ifdef _DEBUG
#define wdebug wprint
#define edebug eprint
#else
#define wdebug(...)
#define edebug(...)
#endif

#endif /* _TYPES_H_ */
