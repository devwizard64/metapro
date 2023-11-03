#ifndef __CPU_H__
#define __CPU_H__

#include "app.h"

#if defined(__NATIVE__) && defined(DEBUG)
#define PATH_ROOT   "app" SEP APP_PATH SEP
#elif defined(__3DS__) && defined(DEBUG)
#define PATH_ROOT   "sdmc:" SEP "3ds" SEP
#else
#define PATH_ROOT   ""
#endif

#define PATH_APP    PATH_ROOT "app.bin"
#define PATH_EEPROM PATH_ROOT "eeprom.bin"
#define PATH_SRAM   PATH_ROOT "sram.bin"
#define PATH_DRAM   PATH_ROOT "dram.bin"
#define PATH_CONT   PATH_ROOT "cont.bin"

#ifdef APP_UCZL
#define CPU_DRAM_SIZE   0xC00000
#else
#define CPU_DRAM_SIZE   0x400000
#endif

#define ARG_F(x)        (*(f32 *)&(x))

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

#if defined(__UNSMJ00_0021D7D0_C__) || \
    defined(__UNSME00_0021F4C0_C__) || \
    defined(__UNSMC03_0020AAF0_C__)
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
#define __ptr(addr)     ((PTR)((u8 *)(addr)-cpu_dram))

#define cpu_ptr(addr)   ((void *)&cpu_dram[__tlb(addr)     ])
#define cpu_s8(addr)    ((s8   *)&cpu_dram[__tlb(addr)^AX_B])
#define cpu_u8(addr)    ((u8   *)&cpu_dram[__tlb(addr)^AX_B])
#define cpu_s16(addr)   ((s16  *)&cpu_dram[__tlb(addr)^AX_H])
#define cpu_u16(addr)   ((u16  *)&cpu_dram[__tlb(addr)^AX_H])
#define cpu_s32(addr)   ((s32  *)&cpu_dram[__tlb(addr)^AX_W])
#define cpu_u32(addr)   ((u32  *)&cpu_dram[__tlb(addr)^AX_W])
#define cpu_f32(addr)   ((f32  *)&cpu_dram[__tlb(addr)^AX_W])

#define __lwl(addr, val)                        \
{                                               \
    PTR _addr = addr;                           \
    int _i = _addr & 3;                         \
    _addr &= ~3;                                \
    val &= cpu_lwl_mask[_i];                    \
    val |= *cpu_u32(_addr) << cpu_l_shift[_i];  \
}
#define __lwr(addr, val)                        \
{                                               \
    PTR _addr = addr;                           \
    int _i = _addr & 3;                         \
    _addr &= ~3;                                \
    val &= cpu_lwr_mask[_i];                    \
    val |= *cpu_u32(_addr) >> cpu_r_shift[_i];  \
}
#define __swl(addr, val)                        \
{                                               \
    PTR _addr = addr;                           \
    u32 _val  = val;                            \
    int _i = _addr & 3;                         \
    _addr &= ~3;                                \
    _val <<= cpu_l_shift[_i];                   \
    _val |= *cpu_u32(_addr) & cpu_swl_mask[_i]; \
    *cpu_u32(_addr) = _val;                     \
}
#define __swr(addr, val)                        \
{                                               \
    PTR _addr = addr;                           \
    u32 _val  = val;                            \
    int _i = _addr & 3;                         \
    _addr &= ~3;                                \
    _val <<= cpu_r_shift[_i];                   \
    _val |= *cpu_u32(_addr) & cpu_swr_mask[_i]; \
    *cpu_u32(_addr) = _val;                     \
}
#define __ld(addr, x)                                       \
{                                                           \
    x = (s64)*cpu_s32((addr)+0) << 32 | *cpu_u32((addr)+4); \
}
#define __sd(addr, x)                       \
{                                           \
    *cpu_s32((addr)+0) = (s64)(x) >> 32;    \
    *cpu_s32((addr)+4) = (s64)(x) >>  0;    \
}
#define __ldc1(addr, x)             \
{                                   \
    x.i[1^IX] = *cpu_s32((addr)+0); \
    x.i[0^IX] = *cpu_s32((addr)+4); \
}
#define __sdc1(addr, x)             \
{                                   \
    *cpu_s32((addr)+0) = x.i[1^IX]; \
    *cpu_s32((addr)+4) = x.i[0^IX]; \
}

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
extern CPU cpu;

extern void __break(int code);
extern void __call(PTR addr);
#ifdef APP_DCALL
extern PTR  __dcall(PTR addr);
#endif
extern void *__nullswap(void *dst, const void *src, u32 size);
extern void *__byteswap(void *dst, const void *src, u32 size);
extern void *__halfswap(void *dst, const void *src, u32 size);
extern void *__wordswap(void *dst, const void *src, u32 size);
extern void cart_rd(void *dst, PTR src, u32 size);
#if EEPROM
extern void eeprom_rd(void *dst, uint src, u32 size);
extern void eeprom_wr(uint dst, const void *src, u32 size);
#endif
#ifdef SRAM
extern void sram_rd(void *dst, PTR src, u32 size);
extern void sram_wr(PTR dst, const void *src, u32 size);
#endif
extern void cpu_init(void);
extern void cpu_exit(void);
extern void cpu_save(void);
extern void cpu_load(void);

#endif /* __CPU_H__ */
