#include "types.h"
#include "cpu.h"

#ifndef LLE

#include "abi.h"

#define asp_u8(addr)  ((u8  *)&asp_dmem[(addr) ^ (AX_H >> 1)])
#define asp_s16(addr) ((s16 *)&asp_dmem[(addr) ^ (AX_W >> 1)])
#define asp_u16(addr) ((u16 *)&asp_dmem[(addr) ^ (AX_W >> 1)])

typedef void ASPCALL(u32 w0, u32 w1);

/*
0x0000 - 0x02C0: asp_data.bin
0x0320: asp_addr_table
0x0360: asp_var
    0x00: asp_dmemin
    0x02: asp_dmemout
    0x04: asp_count
    0x06: asp_vol_l
    0x08: asp_vol_r
    0x0A: asp_aux_0
    0x0C: asp_aux_1
    0x0E: asp_aux_2
    0x10: asp_voltgt_l
    0x12: asp_volrate_l
    0x16: asp_voltgt_r
    0x18: asp_volrate_r
    0x1C: asp_dry_gain
    0x1E: asp_wet_gain
    0x10: asp_loop
0x0380: asp_alist
0x04C0: asp_adpcm
0x05C0: asp_sample
*/

/*
#define asp_addr_table  0x0320
#define asp_dmemin      0x0360
#define asp_demmout     0x0362
#define asp_count       0x0364
#define asp_vol_l       0x0366
#define asp_vol_r       0x0368
#define asp_aux_0       0x036A
#define asp_aux_1       0x036C
#define asp_aux_2       0x036E
#define asp_voltgt_l    0x0370
#define asp_volrate_l   0x0372
#define asp_voltgt_r    0x0376
#define asp_volrate_r   0x0378
#define asp_dry_gain    0x037C
#define asp_wet_gain    0x037E
#define asp_loop        0x0370
*/
#ifdef ASP_MAIN
#define asp_adpcm       0x04C0
#define asp_sample      0x05C0
#endif
#ifdef ASP_NAUDIO
#define asp_adpcm       0x03F0
#define asp_sample      0x04F0
#define asp_dmemin      0x04F0
#define asp_dmemout     0x09D0
#define asp_aux_0       0x0B40
#define asp_aux_1       0x0CB0
#define asp_aux_2       0x0E20

#define asp_count       0x0170
#endif

static u8   asp_dmem[0x1000];
#ifdef ASP_MAIN
static u8  *asp_addr_table[16];
static u16  asp_dmemin;
static u16  asp_dmemout;
static u16  asp_count;
#endif
static s16  asp_vol_l;      /* ? */
static s16  asp_vol_r;      /* ? */
#ifdef ASP_MAIN
static u16  asp_aux_0;
static u16  asp_aux_1;
static u16  asp_aux_2;
#endif
static s16  asp_voltgt_l;   /* ? */
static s32  asp_volrate_l;  /* ? */
static s16  asp_voltgt_r;   /* ? */
static s32  asp_volrate_r;  /* ? */
static s16  asp_dry_gain;   /* ? */
static s16  asp_wet_gain;   /* ? */
static s16 *asp_loop;       /* ? */

static void *asp_addr(PTR addr)
{
#ifdef ASP_MAIN
    return &asp_addr_table[addr >> 24 & 0xF][addr & 0xFFFFFF];
#endif
#ifdef ASP_NAUDIO
    return &cpu_dram[addr];
#endif
}

static void asp_a_spnoop(UNUSED u32 w0, UNUSED u32 w1)
{
}

#include "asp/a_adpcm.c" /* not verified */
#include "asp/a_clearbuff.c"

#ifdef ASP_MAIN2
static void asp_a_addmixer(UNUSED u32 w0, UNUSED u32 w1)
{
    edebug("A_ADDMIXER\n");
}
#else
#include "asp/a_envmixer.c" /* not verified */
#include "asp/a_loadbuff.c"
#include "asp/a_resample.c" /* not verified */
#endif

#include "asp/a_savebuff.c"

#ifdef ASP_NAUDIO
static void asp_a_mp3(UNUSED u32 w0, UNUSED u32 w1)
{
    edebug("A_MP3\n");
}

static void asp_a_mp3addy(UNUSED u32 w0, UNUSED u32 w1)
{
    edebug("A_MP3ADDY\n");
}
#else
#include "asp/a_segment.c"
#include "asp/a_setbuff.c"
#endif

#ifdef ASP_MAIN2
static void asp_a_duplicate(UNUSED u32 w0, UNUSED u32 w1)
{
    edebug("A_DUPLICATE\n");
}
#else
#include "asp/a_setvol.c"
#endif

#include "asp/a_dmemmove.c"
#include "asp/a_loadadpcm.c"
#include "asp/a_mixer.c" /* not verified */
#include "asp/a_interleave.c"

#ifdef ASP_MAIN2
static void asp_a_hilogain(UNUSED u32 w0, UNUSED u32 w1)
{
    edebug("A_HILOGAIN\n");
}
#else
static void asp_a_polef(UNUSED u32 w0, UNUSED u32 w1)
{
}
#endif

#include "asp/a_setloop.c"

#ifdef ASP_MAIN2
static void asp_a_interl(UNUSED u32 w0, UNUSED u32 w1)
{
    edebug("A_INTERL\n");
}

static void asp_a_envsetup1(UNUSED u32 w0, UNUSED u32 w1)
{
    edebug("A_ENVSETUP1\n");
}

#include "asp/a_envmixer.c"
#include "asp/a_loadbuff.c"

static void asp_a_envsetup2(UNUSED u32 w0, UNUSED u32 w1)
{
    edebug("A_ENVSETUP2\n");
}
#endif

static ASPCALL *const asp_table[] =
{
    /* 0x00 */  asp_a_spnoop,
    /* 0x01 */  asp_a_adpcm,
    /* 0x02 */  asp_a_clearbuff,
#ifdef ASP_MAIN2
    /* 0x03 */  asp_a_spnoop,
    /* 0x04 */  asp_a_addmixer,
    /* 0x05 */  asp_a_spnoop,
#else
    /* 0x03 */  asp_a_envmixer,
    /* 0x04 */  asp_a_loadbuff,
    /* 0x05 */  asp_a_resample,
#endif
    /* 0x06 */  asp_a_savebuff,
#ifdef ASP_NAUDIO
    /* 0x07 */  asp_a_mp3,
    /* 0x08 */  asp_a_mp3addy,
#else
    /* 0x07 */  asp_a_segment,
    /* 0x08 */  asp_a_setbuff,
#endif
#ifdef ASP_MAIN2
    /* 0x09 */  asp_a_duplicate,
#else
    /* 0x09 */  asp_a_setvol,
#endif
    /* 0x0A */  asp_a_dmemmove,
    /* 0x0B */  asp_a_loadadpcm,
    /* 0x0C */  asp_a_mixer,
    /* 0x0D */  asp_a_interleave,
#ifdef ASP_MAIN2
    /* 0x0E */  asp_a_hilogain,
#else
    /* 0x0E */  asp_a_polef,
#endif
    /* 0x0F */  asp_a_setloop,
#ifdef ASP_MAIN2
    /* 0x10 */  asp_a_spnoop,
    /* 0x11 */  asp_a_interl,
    /* 0x12 */  asp_a_envsetup1,
    /* 0x13 */  asp_a_envmixer,
    /* 0x14 */  asp_a_loadbuff,
    /* 0x15 */  asp_a_savebuff,
    /* 0x16 */  asp_a_envsetup2,
    /* 0x17 */  asp_a_spnoop,
    /* 0x18 */  asp_a_hilogain,
    /* 0x19 */  asp_a_spnoop,
    /* 0x1A */  asp_a_duplicate,
    /* 0x1B */  asp_a_spnoop,
    /* 0x1C */  asp_a_spnoop,
    /* 0x1D */  asp_a_spnoop,
    /* 0x1E */  asp_a_spnoop,
    /* 0x1F */  asp_a_spnoop,
#endif
};

void asp_update(u32 *al, uint size)
{
#ifdef ASP_NAUDIO
    while (size > 0)
#else
    do
#endif
    {
        u32 w0 = al[0];
        u32 w1 = al[1];
        al += 2;
        if (w0 >> 24 < lenof(asp_table))
        {
            asp_table[w0 >> 24](w0, w1);
        }
        else
        {
            wdebug("invalid Acmd {{0x%08" FMT_X ", 0x%08" FMT_X "}}\n", w0, w1);
        }
        size -= 8;
    }
#ifndef ASP_NAUDIO
    while (size > 0);
#endif
}

#endif
