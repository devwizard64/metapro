#ifndef _CPU_H_
#define _CPU_H_

#ifndef __ASSEMBLER__

#include <stdio.h>
#include <stdlib.h>

#include "types.h"
#include "app.h"
#include "lib.h"

#ifdef GEKKO
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

#ifdef _EB
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

#ifdef APP_UNK4
#define __break(code)
#else
#define __break(code) thread_fault()
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
