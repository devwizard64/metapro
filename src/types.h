#ifndef _TYPES_H_
#define _TYPES_H_

#define false   0
#define true    1

#ifdef WIN32
#define SEP     "\\"
#else
#define SEP     "/"
#endif

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
#ifdef _GCN
#include <gccore.h>
#endif

typedef unsigned int    uint;
typedef float           f32;
typedef double          f64;

#define unused          __attribute__((unused))
#define fallthrough     __attribute__((fallthrough))
#define forceinline     __attribute__((always_inline))
#define lenof(x)        (sizeof((x)) / sizeof((x)[0]))

#ifdef _3DS
#define FMT_d   "ld"
#define FMT_x   "lx"
#define FMT_X   "lX"
#else
#define FMT_d   "d"
#define FMT_x   "x"
#define FMT_X   "X"
#endif

#endif /* __ASSEMBLER__ */

#endif /* _TYPES_H_ */
