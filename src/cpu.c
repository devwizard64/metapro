#include "types.h"
#include "app.h"
#include "cpu.h"
#include "sys.h"
#include "gsp.h"

#include "ultra64.h"

const u32 cpu_lwl_mask[] = {0x00000000, 0x000000FF, 0x0000FFFF, 0x00FFFFFF};
const u32 cpu_lwr_mask[] = {0xFFFFFF00, 0xFFFF0000, 0xFF000000, 0x00000000};
const u32 cpu_swl_mask[] = {0x00000000, 0xFF000000, 0xFFFF0000, 0xFFFFFF00};
const u32 cpu_swr_mask[] = {0x00FFFFFF, 0x0000FFFF, 0x000000FF, 0x00000000};
const u8  cpu_l_shift[]  = { 0,  8, 16, 24};
const u8  cpu_r_shift[]  = {24, 16,  8,  0};

u8 cpu_dram[CPU_DRAM_SIZE];
#if EEPROM_SIZE > 0
u64 eeprom[EEPROM_SIZE];
#endif
struct cpu cpu;

#ifdef APP_DCALL
static PTR cpu_dcall_table[lenof(app_dcall_table)-1];
#endif
#ifdef APP_CACHE
static u8 *cpu_cache_table[lenof(app_cache_table)];
#endif

#if 0
PTR __tlb(PTR addr)
{
#define MAP(base, size) \
    {if (addr >= (base) && addr < (base)+(size)) return addr-(base);}
    MAP(0x80000000, CPU_DRAM_SIZE);
    MAP(0xA0000000, CPU_DRAM_SIZE);
#undef MAP
    pdebug("badvaddr 0x%08X\n", addr);
    return ~0;
}
#endif

void __break(unused uint code)
{
#ifndef APP_UNK4
    if (os_thread != NULL)
    {
        os_event(&os_event_table[OS_EVENT_FAULT]);
        thread_destroy(os_thread);
    }
    else
    {
        exit(EXIT_FAILURE);
    }
#endif
}

void __call(PTR addr)
{
    const struct app_call *start = app_call_table;
    uint len = lenof(app_call_table)-1;
    do
    {
        const struct app_call *call;
        if (len > 1) len /= 2;
        call = start + len;
        if (addr >= call->addr) start = call;
        else if (len == 1) edebug("__call(0x%08" FMT_X "U)\n", addr);
    }
    while (start->addr != addr);
    start->call();
}

#ifdef APP_DCALL
PTR __dcall(PTR addr)
{
    const PTR *app = app_dcall_table;
    PTR       *cpu = cpu_dcall_table;
    addr = __tlb(addr);
    while (addr >= app[1])
    {
        app++;
        cpu++;
    }
    return cpu[0];
}
#endif

void *__nullswap(void *dst, unused const void *src, unused u32 size)
{
    return dst;
}

void *__byteswap(void *dst, const void *src, u32 size)
{
    u8 *_dst = dst;
    const u8 *_src = src;
    uint i;
    while (size >= 4)
    {
        u8 a = _src[0];
        u8 b = _src[1];
        u8 c = _src[2];
        u8 d = _src[3];
        _dst[0] = d;
        _dst[1] = c;
        _dst[2] = b;
        _dst[3] = a;
        _dst += 4;
        _src += 4;
        size -= 4;
    }
    for (i = 0; i < size; i++) _dst[i] = _src[i^3];
    return dst;
}

void *__halfswap(void *dst, const void *src, u32 size)
{
    u8 *_dst = dst;
    const u8 *_src = src;
    uint i;
    while (size >= 4)
    {
        u8 a = _src[0];
        u8 b = _src[1];
        u8 c = _src[2];
        u8 d = _src[3];
        _dst[0] = b;
        _dst[1] = a;
        _dst[2] = d;
        _dst[3] = c;
        _dst += 4;
        _src += 4;
        size -= 4;
    }
    for (i = 0; i < size; i++) _dst[i] = _src[i^2];
    return dst;
}

void *__wordswap(void *dst, const void *src, u32 size)
{
    u16 *_dst = dst;
    const u16 *_src = src;
    uint i;
    while (size >= 4)
    {
        u16 a = _src[0];
        u16 b = _src[0];
        _dst[0] = b;
        _dst[1] = a;
        _dst += 2;
        _src += 2;
        size -= 4;
    }
    for (i = 0; i < size; i++) _dst[i] = _src[i^1];
    return dst;
}

typedef void *CPU_SWAP(void *dst, const void *src, u32 size);
static CPU_SWAP *cpu_swap = __nullswap;

void dma(void *dst, PTR src, u32 size)
{
#if defined(APP_DCALL) || defined(APP_CACHE)
    uint i;
#endif
    FILE *f;
#ifdef APP_DCALL
    for (i = 0; i < lenof(cpu_dcall_table); i++)
    {
        PTR app  = app_dcall_table[i];
        PTR addr = __ptr(dst);
        if (app >= addr && app < addr+size)
        {
            cpu_dcall_table[i] = src;
        }
    }
#endif
#ifdef APP_CACHE
    for (i = 0; i < lenof(cpu_cache_table); i++)
    {
        const struct app_cache *cache = &app_cache_table[i];
        if (src >= cache->addr && src+size <= cache->addr+cache->size)
        {
            memcpy(dst, &cpu_cache_table[i][src-cache->addr], size);
            return;
        }
    }
#endif
    f = fopen(PATH_APP, "rb");
    fseek(f, src, SEEK_SET);
    fread(dst, 1, size, f);
    cpu_swap(dst, dst, size);
    fclose(f);
    gsp_cache();
}

#if EEPROM_SIZE > 0
static void eeprom_read(void)
{
    FILE *f = fopen(PATH_EEPROM, "rb");
    if (f != NULL)
    {
        fread(eeprom, 1, sizeof(eeprom), f);
        fclose(f);
    }
    else
    {
        memset(eeprom, 0xFF, sizeof(eeprom));
    }
}

void eeprom_write(void)
{
    FILE *f = fopen(PATH_EEPROM, "wb");
    if (f != NULL)
    {
        fwrite(eeprom, 1, sizeof(eeprom), f);
        fclose(f);
    }
    else
    {
        wdebug("could not write '" PATH_EEPROM "'\n");
    }
}
#endif

void cpu_init(void)
{
    FILE *f;
    u32 mode;
#ifdef APP_CACHE
    uint i;
#endif
    f = fopen(PATH_APP, "rb");
    if (f == NULL) eprint("could not read '" PATH_APP "'\n");
    fread(&mode, 1, sizeof(mode), f);
    switch (mode)
    {
        case 0x40123780: cpu_swap = __byteswap; break;
        case 0x37804012: cpu_swap = __halfswap; break;
        case 0x12408037: cpu_swap = __wordswap; break;
    }
#ifdef APP_CACHE
    for (i = 0; i < lenof(cpu_cache_table); i++)
    {
        const struct app_cache *cache = &app_cache_table[i];
        u8 *dst;
        fseek(f, cache->addr, SEEK_SET);
        dst = cpu_cache_table[i] = malloc(cache->size);
        fread(dst, 1, cache->size, f);
        cpu_swap(dst, dst, cache->size);
    }
#endif
    fclose(f);
#if EEPROM_SIZE > 0
    eeprom_read();
#endif
}

void cpu_exit(void)
{
#ifdef APP_CACHE
    uint i;
    for (i = 0; i < lenof(cpu_cache_table); i++) free(cpu_cache_table[i]);
#endif
}

void cpu_save(void)
{
    FILE *f = fopen(PATH_DRAM, "wb");
    if (f != NULL)
    {
        fwrite(cpu_dram, 1, sizeof(cpu_dram), f);
    #ifdef APP_DCALL
        fwrite(cpu_dcall_table, 1, sizeof(cpu_dcall_table), f);
    #endif
        fclose(f);
    #if EEPROM_SIZE > 0
        eeprom_write();
    #endif
    }
    else
    {
        wdebug("could not write '" PATH_DRAM "'\n");
    }
}

void cpu_load(void)
{
    FILE *f = fopen(PATH_DRAM, "rb");
    if (f != NULL)
    {
        fread(cpu_dram, 1, sizeof(cpu_dram), f);
    #ifdef APP_DCALL
        fread(cpu_dcall_table, 1, sizeof(cpu_dcall_table), f);
    #endif
        fclose(f);
    #if EEPROM_SIZE > 0
        eeprom_read();
    #endif
        gsp_cache();
    }
    else
    {
        wdebug("could not read '" PATH_DRAM "'\n");
    }
}
