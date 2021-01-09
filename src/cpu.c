#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef GEKKO
#include <fat.h>
#endif

#include "types.h"
#include "cpu.h"
#include "app.h"

#define CPU_MODE_BYTESWAP   0x40123780
#ifdef _EB
#define CPU_MODE_WORDSWAP   0x37804012
#else
#define CPU_MODE_WORDSWAP   0x12408037
#endif

static void cpu_read_native(FILE *, void *, u32);

const u32 cpu_lwl_mask[] = {0x00000000, 0x000000FF, 0x0000FFFF, 0x00FFFFFF};
const u32 cpu_lwr_mask[] = {0xFFFFFF00, 0xFFFF0000, 0xFF000000, 0x00000000};
const u32 cpu_swl_mask[] = {0x00000000, 0xFF000000, 0xFFFF0000, 0xFFFFFF00};
const u32 cpu_swr_mask[] = {0x00FFFFFF, 0x0000FFFF, 0x000000FF, 0x00000000};
const u8  cpu_l_shift[]  = { 0,  8, 16, 24};
const u8  cpu_r_shift[]  = {24, 16,  8,  0};

static void (*cpu_read)(FILE *, void *, u32) = cpu_read_native;

#ifdef APP_DCALL
static u32 cpu_dcall_table[lenof(app_dcall_table)];
#endif
#ifdef APP_CACHE
static u8 *cpu_cache_table[lenof(app_cache_table)];
#endif

/* todo: align */
u8    cpu_dram[CPU_DRAM_SIZE];
reg_t cpu_reg[CPU_REG_LEN];

void __call(u32 addr)
{
    const struct call_t *start = app_call_table;
    uint len = lenof(app_call_table);
    do
    {
        const struct call_t *call;
    #ifdef _DEBUG
        if (addr < start->addr)
        {
            fprintf(stderr, "error: __call(0x%08" FMT_X "U)\n", addr);
            exit(EXIT_FAILURE);
        }
    #endif
        len = (len+1) / 2;
        call = start + len;
        if (addr >= call->addr)
        {
            start = call;
        }
    }
    while (start->addr != addr);
    start->call();
}

#ifdef APP_DCALL
u32 __dcall(u32 addr)
{
    const u32 *app;
    u32       *cpu;
    addr = (u8 *)__tlb(addr) - cpu_dram;
    app = app_dcall_table;
    cpu = cpu_dcall_table;
    while (addr >= app[1])
    {
        app++;
        cpu++;
    }
    return cpu[0];
}
#endif

void __byteswap(void *dst, const void *src, s32 size)
{
    u8 *_dst = dst;
    const u8 *_src = src;
    do
    {
        _dst[0] = _src[3];
        _dst[1] = _src[2];
        _dst[2] = _src[1];
        _dst[3] = _src[0];
        _dst += 4;
        _src += 4;
        size -= 4;
    }
    while (size > 0);
}

void __wordswap(void *dst, const void *src, s32 size)
{
#ifdef _EB
    u8 *_dst = dst;
    const u8 *_src = src;
    do
    {
        _dst[0] = _src[1];
        _dst[1] = _src[0];
        _dst[2] = _src[3];
        _dst[3] = _src[2];
        _dst += 4;
        _src += 4;
        size -= 4;
    }
    while (size > 0);
#else
    u16 *_dst = dst;
    const u16 *_src = src;
    do
    {
        _dst[0] = _src[1];
        _dst[1] = _src[0];
        _dst += 2;
        _src += 2;
        size -= 4;
    }
    while (size > 0);
#endif
}

static void cpu_read_native(FILE *f, void *dst, u32 size)
{
    fread(dst, 1, size, f);
}

static void cpu_read_byteswap(FILE *f, void *dst, u32 size)
{
    void *data = malloc(size);
    fread(data, 1, size, f);
    __byteswap(dst, data, size);
    free(data);
}

static void cpu_read_wordswap(FILE *f, void *dst, u32 size)
{
    void *data = malloc(size);
    fread(data, 1, size, f);
    __wordswap(dst, data, size);
    free(data);
}

void __dma(void *dst, u32 src, u32 size)
{
#if defined(APP_DCALL) || defined(APP_CACHE)
    uint i;
#endif
    FILE *f;
#ifdef APP_DCALL
    for (i = 0; i < lenof(app_dcall_table); i++)
    {
        u32 app  = app_dcall_table[i];
        u32 addr = (u8 *)dst - cpu_dram;
        if (app >= addr && app < addr+size)
        {
            cpu_dcall_table[i] = src;
        }
    }
#endif
    /* $ */
    /*
    if (src == 0x007CC1D0)
    {
        f = fopen("kirby.seq", "rb");
        fseek(f, 0, SEEK_END);
        size = ftell(f);
        fseek(f, 0, SEEK_SET);
        cpu_read_byteswap(f, dst, size);
        fclose(f);
        return;
    }
    */
#ifdef APP_CACHE
    for (i = 0; i < lenof(app_cache_table); i++)
    {
        const struct cache_t *cache = &app_cache_table[i];
        if (src >= cache->addr && src+size <= cache->addr+cache->size)
        {
            memcpy(dst, &cpu_cache_table[i][src-cache->addr], size);
            return;
        }
    }
#endif
    f = fopen(PATH_APP, "rb");
    fseek(f, src, SEEK_SET);
    cpu_read(f, dst, size);
    fclose(f);
    /* $ */
    /*
    if (src == 0x007CC620)
    {
        ((u8 *)dst)[(0x007CC6B4-0x007CC620)^3] = 0x0D;
    }
    */
}

void __eeprom_read(void *data, uint size)
{
    FILE *f = fopen(PATH_EEPROM, "rb");
    if (f != NULL)
    {
        fread(data, 1, size, f);
        fclose(f);
    }
    else
    {
        memset(data, 0xFF, size);
    }
}

void __eeprom_write(const void *data, uint size)
{
    FILE *f = fopen(PATH_EEPROM, "wb");
    if (f != NULL)
    {
        fwrite(data, 1, size, f);
        fclose(f);
    }
    else
    {
    #ifdef _DEBUG
        fprintf(stderr, "warning: could not write '" PATH_EEPROM "'\n");
    #endif
    }
}

void cpu_init(void)
{
    FILE *f;
    u32   mode;
#ifdef APP_CACHE
    uint  i;
#endif
#ifdef _3DS
    osSetSpeedupEnable(true);
    romfsInit();
#endif
#ifdef GEKKO
    fatInitDefault();
#endif
    f = fopen(PATH_APP, "rb");
    if (f == NULL)
    {
        fprintf(stderr, "error: could not read '" PATH_APP "'\n");
    #ifdef _3DS
        svcSleepThread(3000000000LL);
    #endif
        exit(EXIT_FAILURE);
    }
    fread(&mode, 1, sizeof(mode), f);
    switch (mode)
    {
        case CPU_MODE_BYTESWAP: cpu_read = cpu_read_byteswap; break;
        case CPU_MODE_WORDSWAP: cpu_read = cpu_read_wordswap; break;
    }
#ifdef APP_CACHE
    for (i = 0; i < lenof(app_cache_table); i++)
    {
        const struct cache_t *cache = &app_cache_table[i];
        fseek(f, cache->addr, SEEK_SET);
        cpu_cache_table[i] = malloc(cache->size);
        cpu_read(f, cpu_cache_table[i], cache->size);
    }
#endif
    fclose(f);
}

void cpu_destroy(void)
{
#ifdef _3DS
    romfsExit();
#endif
}
