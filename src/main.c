#include "types.h"
#include "app.h"
#include "demo.h"
#include "sys.h"
#include "cpu.h"

#include "ultra64.h"

static void start(void)
{
    dma(&cpu_dram[0], 0x4C0, 0x2B4);
    *__u32(0x80000300) = OS_TV_NTSC;
    dma(__dram(APP_ENTRY), 0x1000, 0x100000);
    memset(__dram(APP_BSS_ADDR), 0x00, APP_BSS_SIZE);
    sp = (s32)APP_STACK;
    app_main();
}

int main(unused int argc, unused char *argv[])
{
    sys_init();
    demo_main();
    sys_main(start);
    return EXIT_SUCCESS;
}
