#include "types.h"
#include "app.h"
#include "demo.h"
#include "cpu.h"
#include "sys.h"

#include "ultra64.h"

static void start(void)
{
    dma(__dram(0x80000000), 0x000004C0, 0x000002B4);
    *__u32(0x80000300) = OS_TV_NTSC;
    dma(__dram(APP_ENTRY), 0x00001000, 0x00100000);
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
