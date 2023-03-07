#include "types.h"
#include "app.h"
#include "demo.h"
#include "sys.h"
#include "cpu.h"

#include "ultra64.h"

extern void seq_init(int argc, char *argv[]);

static void start(void)
{
    dma(&cpu_dram[0], 0x4C0, 0x2B4);
    dma(cpu_ptr(APP_ENTRY), 0x1000, 0x100000);
    *cpu_u32(0x80000300) = OS_TV_NTSC;
    *cpu_u32(0x80000304) = 0;
    *cpu_u32(0x80000308) = 0xB0000000;
    *cpu_u32(0x8000030C) = 0;
    *cpu_u32(0x80000314) = 0;
    *cpu_u32(0x80000318) = MIN(0x800000, CPU_DRAM_SIZE);
    memset(cpu_ptr(APP_BSS_ADDR), 0, APP_BSS_SIZE);
    sp = (s32)APP_STACK;
    app_main();
}

int main(int argc, char *argv[])
{
#ifdef APP_SEQ
    seq_init(argc, argv);
#else
    (void)argc;
    (void)argv;
#endif
    sys_init();
    demo_main();
    sys_main(start);
    return 0;
}
