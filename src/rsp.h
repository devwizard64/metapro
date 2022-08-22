#ifndef __RSP_H__
#define __RSP_H__

#include "ultra64.h"

typedef union vreg
{
    s8  b[16];
    s16 s[8];
    u16 u[8];
}
VREG;

typedef struct rsp
{
    s32 reg[32];
    VREG vreg[32];
    s64 acc[8];
    u16 vcc;
    u16 vco;
    u8  vce;
    s8  bcode;
    u16 baddr;
    u16 pc;
    u16 mem_addr;
    PTR dram_addr;
    u32 rd_len;
    u32 wr_len;
}
RSP;

extern void rsp_main(OSTask *task);

extern void gsp_init(void);
extern void gsp_exit(void);
extern void gsp_cache(void);
extern void gsp_update(PTR ucode, u32 *dl);
extern void gsp_image(void *img);

extern void asp_update(u32 *al, uint size);

#endif /* __RSP_H__ */
