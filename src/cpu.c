#include "types.h"
#include "app.h"
#include "sys.h"
#include "cpu.h"
#include "rsp.h"

#include "ultra64.h"

const u32 cpu_lwl_mask[] = {0x00000000, 0x000000FF, 0x0000FFFF, 0x00FFFFFF};
const u32 cpu_lwr_mask[] = {0xFFFFFF00, 0xFFFF0000, 0xFF000000, 0x00000000};
const u32 cpu_swl_mask[] = {0x00000000, 0xFF000000, 0xFFFF0000, 0xFFFFFF00};
const u32 cpu_swr_mask[] = {0x00FFFFFF, 0x0000FFFF, 0x000000FF, 0x00000000};
const u8  cpu_l_shift[]  = { 0,  8, 16, 24};
const u8  cpu_r_shift[]  = {24, 16,  8,  0};

u8 cpu_dram[CPU_DRAM_SIZE];
CPU cpu;

#ifdef APP_DCALL
static PTR dcall_ptr[lenof(dcall_table)-1];
#endif
#ifdef APP_CACHE
static u8 *cache_ptr[lenof(cache_table)];
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

void __break(UNUSED int code)
{
#ifndef APP_UNK4
	if (os_thread)
	{
		os_event(&__osEventStateTab[OS_EVENT_CPU_BREAK]);
		th_stop(os_thread);
	}
	else
	{
		exit(1);
	}
#endif
}

void __call(PTR addr)
{
	const CALL *start = call_table;
	uint len = lenof(call_table)-1;
	do
	{
		const CALL *call;
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
	const PTR *app = dcall_table;
	PTR       *cpu = dcall_ptr;
	addr = __tlb(addr);
	while (addr >= app[1])
	{
		app++;
		cpu++;
	}
	return cpu[0];
}
#endif

void *__nullswap(void *dst, UNUSED const void *src, UNUSED u32 size)
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
		u16 b = _src[1];
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

void cart_rd(void *dst, PTR src, u32 size)
{
#if defined(APP_DCALL) || defined(APP_CACHE)
	uint i;
#endif
	FILE *fp;
#ifdef APP_DCALL
	for (i = 0; i < lenof(dcall_ptr); i++)
	{
		PTR app  = dcall_table[i];
		PTR addr = __ptr(dst);
		if (app >= addr && app < addr+size) dcall_ptr[i] = src;
	}
#endif
#ifdef APP_CACHE
	for (i = 0; i < lenof(cache_ptr); i++)
	{
		const CACHE *cache = &cache_table[i];
		if (src >= cache->addr && src+size <= cache->addr+cache->size)
		{
			memcpy(dst, &cache_ptr[i][src-cache->addr], size);
			return;
		}
	}
#endif
	if ((fp = fopen(PATH_APP, "rb")))
	{
		fseek(fp, src, SEEK_SET);
		fread(dst, 1, size, fp);
		fclose(fp);
		cpu_swap(dst, dst, size);
	}
	else
	{
		eprint("could not read '%s'\n", PATH_APP);
	}
#if defined(GSP_F3D) || defined(GSP_F3DEX2)
	gsp_cache();
#endif
}

#if EEPROM
void eeprom_rd(void *dst, uint src, u32 size)
{
	FILE *fp;
	if ((fp = fopen(PATH_EEPROM, "rb")))
	{
		fseek(fp, 8*src, SEEK_SET);
		fread(dst, 1, size, fp);
		fclose(fp);
		byteswap(dst, dst, size);
	}
	else
	{
		memset(dst, ~0, size);
	}
}

void eeprom_wr(uint dst, const void *src, u32 size)
{
	FILE *fp;
	if (
		(fp = fopen(PATH_EEPROM, "r+b")) ||
		(fp = fopen(PATH_EEPROM, "wb"))
	)
	{
		char *data = malloc(size);
		byteswap(data, src, size);
		fseek(fp, 8*dst, SEEK_SET);
		fwrite(data, 1, size, fp);
		fclose(fp);
		free(data);
	}
	else
	{
		wdebug("could not write '%s'\n", PATH_EEPROM);
	}
}
#endif

#ifdef SRAM
void sram_rd(void *dst, PTR src, u32 size)
{
	FILE *fp;
	if ((fp = fopen(PATH_SRAM, "rb")))
	{
		fseek(fp, src, SEEK_SET);
		fread(dst, 1, size, fp);
		fclose(fp);
	}
	else
	{
		memset(dst, ~0, size);
	}
}

void sram_wr(PTR dst, const void *src, u32 size)
{
	FILE *fp;
	if (
		(fp = fopen(PATH_SRAM, "r+b")) ||
		(fp = fopen(PATH_SRAM, "wb"))
	)
	{
		fseek(fp, dst, SEEK_SET);
		fwrite(src, 1, size, fp);
		fclose(fp);
	}
	else
	{
		wdebug("could not write '%s'\n", PATH_SRAM);
	}
}
#endif

void cpu_init(void)
{
	FILE *fp;
	if ((fp = fopen(PATH_APP, "rb")))
	{
#ifdef APP_CACHE
		uint i;
#endif
		u32 mode;
		fread(&mode, 1, 4, fp);
		switch (mode)
		{
		case 0x40123780: cpu_swap = __byteswap; break;
		case 0x37804012: cpu_swap = __halfswap; break;
		case 0x12408037: cpu_swap = __wordswap; break;
		}
#ifdef APP_CACHE
		for (i = 0; i < lenof(cache_ptr); i++)
		{
			const CACHE *cache = &cache_table[i];
			fseek(fp, cache->addr, SEEK_SET);
			cache_ptr[i] = malloc(cache->size);
			fread(cache_ptr[i], 1, cache->size, fp);
			cpu_swap(cache_ptr[i], cache_ptr[i], cache->size);
		}
#endif
		fclose(fp);
	}
	else
	{
		eprint("could not read '%s'\n", PATH_APP);
	}
}

void cpu_exit(void)
{
#ifdef APP_CACHE
	uint i;
	for (i = 0; i < lenof(cache_ptr); i++) free(cache_ptr[i]);
#endif
}

void cpu_save(void)
{
	FILE *fp;
	if ((fp = fopen(PATH_DRAM, "wb")))
	{
		fwrite(cpu_dram, 1, sizeof(cpu_dram), fp);
#ifdef APP_DCALL
		fwrite(dcall_ptr, 1, sizeof(dcall_ptr), fp);
#endif
		fclose(fp);
	}
	else
	{
		wdebug("could not write '%s'\n", PATH_DRAM);
	}
}

void cpu_load(void)
{
	FILE *fp;
	if ((fp = fopen(PATH_DRAM, "rb")))
	{
		fread(cpu_dram, 1, sizeof(cpu_dram), fp);
#ifdef APP_DCALL
		fread(dcall_ptr, 1, sizeof(dcall_ptr), fp);
#endif
		fclose(fp);
#if defined(GSP_F3D) || defined(GSP_F3DEX2)
		gsp_cache();
#endif
	}
	else
	{
		wdebug("could not read '%s'\n", PATH_DRAM);
	}
}
