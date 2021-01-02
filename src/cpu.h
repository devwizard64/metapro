#ifndef _CPU_H_
#define _CPU_H_

#ifndef __ASSEMBLER__

#include <stdio.h>
#include <stdlib.h>

#include "types.h"

struct call_t
{
    u32    addr;
    void (*call)(void);
};

struct cache_t
{
    u32 addr;
    u32 size;
};

#include "lib.h"
#include "app.h"

#ifdef _GCN
#define PATH_START  "cardb:" SEP
#elif defined(_3DS) && defined(_DEBUG)
#define PATH_START  "sdmc:" SEP "3ds" SEP
#else
#define PATH_START  ""
#endif

#ifdef _NATIVE
#define PATH_ROOT   PATH_START "app" SEP APP_PATH SEP
#else
#define PATH_ROOT   PATH_START
#endif

#define PATH_APP    PATH_ROOT  "app.bin"
#define PATH_EEPROM PATH_ROOT  "eeprom.bin"
#define PATH_DRAM   PATH_ROOT  "dram.bin"
#define PATH_INPUT  PATH_ROOT  "input.bin"
#define PATH_CONFIG PATH_START "config.bin"

#ifdef _GCN
#define IX      1
#define AX_B    0
#define AX_H    0
#define __BYTESWAP      memcpy
#define __WORDSWAP      memcpy
#else
#define IX      0
#define AX_B    3
#define AX_H    2
#define __BYTESWAP      __byteswap
#define __WORDSWAP      __wordswap
#endif
#define AX_W    0

#define CPU_DRAM_SIZE   0x00400000

#define R_V0            0x00
#define R_V1            0x01
#define R_A0            0x02
#define R_A1            0x03
#define R_A2            0x04
#define R_A3            0x05
#define R_SP            0x06
#define R_F0            0x07
#define R_F12           0x08
#define R_F14           0x09
#define v0              cpu_reg[R_V0]
#define v1              cpu_reg[R_V1]
#define a0              cpu_reg[R_A0]
#define a1              cpu_reg[R_A1]
#define a2              cpu_reg[R_A2]
#define a3              cpu_reg[R_A3]
#define sp              cpu_reg[R_SP]
#define f0              cpu_reg[R_F0]
#define f12             cpu_reg[R_F12]
#define f14             cpu_reg[R_F14]

#ifdef APP_UNSM
#define CPU_REG_LEN     0x0A
#endif

#ifdef APP_UNK4
#define R_T0            0x0A
#define R_T1            0x0B
#define R_T2            0x0C
#define R_T3            0x0D
#define R_T4            0x0E
#define R_T5            0x0F
#define R_T6            0x10
#define R_T7            0x11
#define R_S0            0x12
#define R_S1            0x13
#define R_S2            0x14
#define R_S3            0x15
#define R_S4            0x16
#define R_S5            0x17
#define R_S6            0x18
#define R_S7            0x19
#define R_T8            0x1A
#define R_T9            0x1B
#define R_S8            0x1C
#define R_RA            0x1D
#define R_LO            0x1E
#define R_HI            0x1F
#define R_F2            0x20
#define R_F4            0x21
#define R_F6            0x22
#define R_F8            0x23
#define R_F10           0x24
#define R_F16           0x25
#define R_F18           0x26
#define R_F20           0x27
#define R_F22           0x28
#define R_F24           0x29
#define R_F26           0x2A
#define R_F28           0x2B
#define R_F30           0x2C
#define CPU_REG_LEN     0x2D

#define t0              cpu_reg[R_T0]
#define t1              cpu_reg[R_T1]
#define t2              cpu_reg[R_T2]
#define t3              cpu_reg[R_T3]
#define t4              cpu_reg[R_T4]
#define t5              cpu_reg[R_T5]
#define t6              cpu_reg[R_T6]
#define t7              cpu_reg[R_T7]
#define s0              cpu_reg[R_S0]
#define s1              cpu_reg[R_S1]
#define s2              cpu_reg[R_S2]
#define s3              cpu_reg[R_S3]
#define s4              cpu_reg[R_S4]
#define s5              cpu_reg[R_S5]
#define s6              cpu_reg[R_S6]
#define s7              cpu_reg[R_S7]
#define t8              cpu_reg[R_T8]
#define t9              cpu_reg[R_T9]
#define fp              cpu_reg[R_S8]
#define ra              cpu_reg[R_RA]
#define lo              cpu_reg[R_LO]
#define hi              cpu_reg[R_HI]
#define f2              cpu_reg[R_F2]
#define f4              cpu_reg[R_F4]
#define f6              cpu_reg[R_F6]
#define f8              cpu_reg[R_F8]
#define f10             cpu_reg[R_F10]
#define f16             cpu_reg[R_F16]
#define f18             cpu_reg[R_F18]
#define f20             cpu_reg[R_F20]
#define f22             cpu_reg[R_F22]
#define f24             cpu_reg[R_F24]
#define f26             cpu_reg[R_F26]
#define f28             cpu_reg[R_F28]
#define f30             cpu_reg[R_F30]
#endif

typedef union
{
    s32 i[2];
    u32 iu[2];
    s64 ll;
    u64 llu;
    f32 f[2];
    f64 d;
}
reg_t;

extern const u32 cpu_lwl_mask[];
extern const u32 cpu_lwr_mask[];
extern const u32 cpu_swl_mask[];
extern const u32 cpu_swr_mask[];
extern const u8  cpu_l_shift[];
extern const u8  cpu_r_shift[];

extern u8    cpu_dram[CPU_DRAM_SIZE];
extern reg_t cpu_reg[CPU_REG_LEN];

#if defined(_UNSME0_0021F4C0_C_) || defined(_UNSMC3_0020AAF0_C_)
static inline void *__tlb(u32 addr)
{
    if (addr >= 0x04000000 && addr < 0x04040000)
    {
        return &cpu_dram[addr - (0x04000000-0x00390000)];
    }
    return &cpu_dram[addr & 0x1FFFFFFF];
}
#else
#define __tlb(addr) ((void *)&cpu_dram[(u32)(addr) & 0x1FFFFFFF])
#endif

#define __read_s8(addr)  (*(s8  *)__tlb((addr) ^ AX_B))
#define __read_u8(addr)  (*(u8  *)__tlb((addr) ^ AX_B))
#define __read_s16(addr) (*(s16 *)__tlb((addr) ^ AX_H))
#define __read_u16(addr) (*(u16 *)__tlb((addr) ^ AX_H))
#define __read_s32(addr) (*(s32 *)__tlb((addr) ^ AX_W))
#define __read_u32(addr) (*(u32 *)__tlb((addr) ^ AX_W))
#define __read_f32(addr) (*(f32 *)__tlb((addr) ^ AX_W))
#define __read_s64(addr) \
    ((s64)__read_s32((addr)+0x00) << 32 | __read_u32((addr)+0x04))
#define __read_u64(addr) \
    ((u64)__read_u32((addr)+0x00) << 32 | __read_u32((addr)+0x04))

#define __write_u8(addr, val)  {*(u8  *)__tlb((addr) ^ AX_B) = (u8)(val);}
#define __write_u16(addr, val) {*(u16 *)__tlb((addr) ^ AX_H) = (u16)(val);}
#define __write_u32(addr, val) {*(u32 *)__tlb((addr) ^ AX_W) = (u32)(val);}
#define __write_f32(addr, val) {*(f32 *)__tlb((addr) ^ AX_W) = (f32)(val);}
#define __write_u64(addr, val)                                                 \
{                                                                              \
    __write_u32((addr)+0x00, (u64)(val) >> 32);                                \
    __write_u32((addr)+0x04, (u64)(val) >>  0);                                \
}
#define __read_u32_l(addr, val)                                                \
{                                                                              \
    u32  _addr = addr;                                                         \
    u32 *_val  = val;                                                          \
    uint _i    = _addr & 0x03;                                                 \
    _addr &= ~0x03;                                                            \
    *_val &= cpu_lwl_mask[_i];                                                 \
    *_val |= __read_u32(_addr) << cpu_l_shift[_i];                             \
}
#define __read_u32_r(addr, val)                                                \
{                                                                              \
    u32  _addr = addr;                                                         \
    u32 *_val  = val;                                                          \
    uint _i    = _addr & 0x03;                                                 \
    _addr &= ~0x03;                                                            \
    *_val &= cpu_lwr_mask[_i];                                                 \
    *_val |= __read_u32(_addr) >> cpu_r_shift[_i];                             \
}
#define __write_u32_l(addr, val)                                               \
{                                                                              \
    u32  _addr = addr;                                                         \
    u32  _val  = val;                                                          \
    uint _i    = _addr & 0x03;                                                 \
    _addr &= ~0x03;                                                            \
    _val <<= cpu_l_shift[_i];                                                  \
    _val |= __read_u32(_addr) & cpu_swl_mask[_i];                              \
    __write_u32(_addr, _val);                                                  \
}
#define __write_u32_r(addr, val)                                               \
{                                                                              \
    u32  _addr = addr;                                                         \
    u32  _val  = val;                                                          \
    uint _i    = _addr & 0x03;                                                 \
    _addr &= ~0x03;                                                            \
    _val <<= cpu_r_shift[_i];                                                  \
    _val |= __read_u32(_addr) & cpu_swr_mask[_i];                              \
    __write_u32(_addr, _val);                                                  \
}

#ifdef APP_UNSM
#define __break(code) thread_fault()
#endif
#ifdef APP_UNK4
#define __break(code)
#endif

extern void __call(u32);
extern u32  __dcall(u32);
extern void __byteswap(void *, const void *, s32);
extern void __wordswap(void *, const void *, s32);
extern void __dma(void *, u32, u32);
extern void __eeprom_read(void *, uint);
extern void __eeprom_write(const void *, uint);
extern void cpu_init(void);
extern void cpu_destroy(void);

#endif /* __ASSEMBLER__ */

#endif /* _CPU_H_ */
