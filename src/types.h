#ifndef __TYPES_H__
#define __TYPES_H__

#define FALSE   0
#define TRUE    1

#define MIN(a, b)               ((a) < (b) ? (a) : (b))
#define MAX(a, b)               ((a) > (b) ? (a) : (b))

#ifdef WIN32
#define SEP     "\\"
#else
#define SEP     "/"
#endif

#if defined(__NDS__) || defined(__3DS__)
#define FMT_d   "ld"
#define FMT_u   "lu"
#define FMT_x   "lx"
#define FMT_X   "lX"
#else
#define FMT_d   "d"
#define FMT_u   "u"
#define FMT_x   "x"
#define FMT_X   "X"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <setjmp.h>

#include <math.h>

#ifdef __NATIVE__
#include <stdint.h>
#include <time.h>

#include <SDL2/SDL.h>
#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#include <GL/glext.h>
#endif

typedef  int8_t  s8;
typedef uint8_t  u8;
typedef  int16_t s16;
typedef uint16_t u16;
typedef  int32_t s32;
typedef uint32_t u32;
typedef  int64_t s64;
typedef uint64_t u64;
typedef char     bool;
#endif

#ifdef GEKKO
#include <malloc.h>
#include <gccore.h>
#include <fat.h>
#endif

#ifdef __NDS__
#include <nds.h>
#include <fat.h>
#endif

#ifdef __3DS__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#include <3ds.h>
#pragma GCC diagnostic pop
#include <GL/gl.h>
#include <GL/picaGL.h>
#define glVertex3s(x, y, z) glVertex3f(x, y, z)
#endif

typedef unsigned int    uint;
typedef float           f32;
typedef double          f64;

#ifdef __GNUC__
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#define __EB__
#endif
#define UNUSED          __attribute__((unused))
#else
#define UNUSED
#endif
#define lenof(x)        (sizeof((x)) / sizeof((x)[0]))

#define NULLPTR 0
typedef u32 PTR;

extern void eexit(void);
#define wprint(...) fprintf(stderr, "warning: " __VA_ARGS__)
#define eprint(...) {fprintf(stderr, "error: " __VA_ARGS__); eexit();}
#ifdef DEBUG
#define pdebug printf
#define wdebug wprint
#define edebug eprint
#else
#define pdebug(...) {}
#define wdebug(...) {}
#define edebug(...) {}
#endif

#endif /* __TYPES_H__ */
