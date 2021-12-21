#ifndef __CPU_H__
#define __CPU_H__

#include "types.h"
#include "app.h"

#ifndef __ASSEMBLER__

#if defined(__NATIVE__) && defined(__DEBUG__)
#define PATH_ROOT   "app" SEP APP_PATH SEP
#elif defined(__3DS__) && defined(__DEBUG__)
#define PATH_ROOT   "sdmc:" SEP "3ds" SEP
#else
#define PATH_ROOT   ""
#endif

#define PATH_APP    PATH_ROOT "app.bin"
#define PATH_EEPROM PATH_ROOT "eeprom.bin"
#define PATH_DRAM   PATH_ROOT "dram.bin"
#define PATH_INPUT  PATH_ROOT "input.bin"

#ifdef __EB__
#define IX      1
#define AX_B    0
#define AX_H    0
#define byteswap        memmove
#define wordswap        memmove
#else
#define IX      0
#define AX_B    3
#define AX_H    2
#define byteswap        __byteswap
#define wordswap        __wordswap
#endif
#define AX_W    0

#ifdef APP_UCZL
#define CPU_DRAM_SIZE   0xC00000
#else
#define CPU_DRAM_SIZE   0x400000
#endif

#define EEPROM_SIZE     (64*EEPROM_TYPE*EEPROM_TYPE)

#define ARG_F(x)        (*((f32 *)&(x)))

#if defined(__UNSME0_0021F4C0_C__) || defined(__UNSMC3_0020AAF0_C__)
#define __tlb(addr)                                             \
(                                                               \
    ((PTR)(addr) >= 0x04000000U && (PTR)(addr) < 0x04040000U) ? \
    ((PTR)(addr) - (0x04000000U-0x00390000U)) :                 \
    ((PTR)(addr) & 0x1FFFFFFF)                                  \
)
#else
#if 0
extern PTR __tlb(PTR addr);
#else
#define __tlb(addr) ((PTR)(addr) & 0x1FFFFFFF)
#endif
#endif
#define __dram(addr)    ((void *)&cpu_dram[__tlb(addr)])
#define __ptr(addr)     ((PTR)((u8 *)(addr)-cpu_dram))

#undef __f32
#define __s8(addr)  ((s8  *)&cpu_dram[__tlb(addr)^AX_B])
#define __u8(addr)  ((u8  *)&cpu_dram[__tlb(addr)^AX_B])
#define __s16(addr) ((s16 *)&cpu_dram[__tlb(addr)^AX_H])
#define __u16(addr) ((u16 *)&cpu_dram[__tlb(addr)^AX_H])
#define __s32(addr) ((s32 *)&cpu_dram[__tlb(addr)^AX_W])
#define __u32(addr) ((u32 *)&cpu_dram[__tlb(addr)^AX_W])
#define __f32(addr) ((f32 *)&cpu_dram[__tlb(addr)^AX_W])

#define __lwl(addr, val)                        \
{                                               \
    PTR  _addr = addr;                          \
    uint _i    = _addr & 3;                     \
    _addr &= ~3;                                \
    val &= cpu_lwl_mask[_i];                    \
    val |= *__u32(_addr) << cpu_l_shift[_i];    \
}
#define __lwr(addr, val)                        \
{                                               \
    PTR  _addr = addr;                          \
    uint _i    = _addr & 3;                     \
    _addr &= ~3;                                \
    val &= cpu_lwr_mask[_i];                    \
    val |= *__u32(_addr) >> cpu_r_shift[_i];    \
}
#define __swl(addr, val)                        \
{                                               \
    PTR  _addr = addr;                          \
    u32  _val  = val;                           \
    uint _i    = _addr & 3;                     \
    _addr &= ~3;                                \
    _val <<= cpu_l_shift[_i];                   \
    _val |= *__u32(_addr) & cpu_swl_mask[_i];   \
    *__u32(_addr) = _val;                       \
}
#define __swr(addr, val)                        \
{                                               \
    PTR  _addr = addr;                          \
    u32  _val  = val;                           \
    uint _i    = _addr & 3;                     \
    _addr &= ~3;                                \
    _val <<= cpu_r_shift[_i];                   \
    _val |= *__u32(_addr) & cpu_swr_mask[_i];   \
    *__u32(_addr) = _val;                       \
}
#define __ld(addr, x)                                   \
{                                                       \
    x = (s64)*__s32((addr)+0) << 32 | *__u32((addr)+4); \
}
#define __sd(addr, x)                   \
{                                       \
    *__s32((addr)+0) = (s64)(x) >> 32;  \
    *__s32((addr)+4) = (s64)(x) >>  0;  \
}
#define __ldc1(addr, x)             \
{                                   \
    x.i[1^IX] = *__s32((addr)+0);   \
    x.i[0^IX] = *__s32((addr)+4);   \
}
#define __sdc1(addr, x)             \
{                                   \
    *__s32((addr)+0) = x.i[1^IX];   \
    *__s32((addr)+4) = x.i[0^IX];   \
}

#ifdef __EB__
#define __str_r(dst, src)   strcpy(dst, __dram(src))
#define __str_w(dst, src)   strcpy(__dram(dst), src)
#else
#define __str_r(dst, src)   \
{                           \
    char *_dst = dst;       \
    PTR   _src = src;       \
    char  _c;               \
    do                      \
    {                       \
        _c = *__s8(_src++); \
        *_dst++ = _c;       \
    }                       \
    while (_c != 0);        \
}
#define __str_w(dst, src)   \
{                           \
    PTR   _dst = dst;       \
    char *_src = src;       \
    char  _c;               \
    do                      \
    {                       \
        _c = *_src++;       \
        *__s8(_dst++) = _c; \
    }                       \
    while (_c != 0);        \
}
#endif

typedef union reg
{
    s32 i[2];
    u32 iu[2];
    s64 ll;
    u64 llu;
    f32 f[2];
    f64 d;
}
REG;

typedef struct cpu
{
#if CPU_ARG_LEN > 0
    s32 arg[CPU_ARG_LEN];
#endif
#if CPU_EXT_LEN > 0
    s64 ext[CPU_EXT_LEN];
#endif
#if CPU_REG_LEN > 0
    REG reg[CPU_REG_LEN];
#endif
}
CPU;

extern const u32 cpu_lwl_mask[];
extern const u32 cpu_lwr_mask[];
extern const u32 cpu_swl_mask[];
extern const u32 cpu_swr_mask[];
extern const u8  cpu_l_shift[];
extern const u8  cpu_r_shift[];

extern u8 cpu_dram[CPU_DRAM_SIZE];
#if EEPROM_SIZE > 0
extern u64 eeprom[EEPROM_SIZE];
#endif
extern CPU cpu;

extern void __break(uint code);
extern void __call(PTR addr);
#ifdef APP_DCALL
extern u32  __dcall(PTR addr);
#endif
extern void *__nullswap(void *dst, const void *src, u32 size);
extern void *__byteswap(void *dst, const void *src, u32 size);
extern void *__halfswap(void *dst, const void *src, u32 size);
extern void *__wordswap(void *dst, const void *src, u32 size);
extern void dma(void *dst, PTR src, u32 size);
#if EEPROM_SIZE > 0
extern void eeprom_write(void);
#endif
extern void cpu_init(void);
extern void cpu_exit(void);
extern void cpu_save(void);
extern void cpu_load(void);

#endif /* __ASSEMBLER__ */

#endif /* __CPU_H__ */
