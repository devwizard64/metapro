#include <stdlib.h>
#include <string.h>

#include "types.h"
#include "demo.h"
#include "cpu.h"
#include "lib.h"
#include "app.h"

#include "ultra64.h"

int main(void)
{
    lib_init();
    demo_main();
    __dma(__tlb(0x80000000), 0x000004C0, 0x000002B0);
    __dma(__tlb(APP_ENTRY), 0x00001000, 0x00100000);
    __write_u32(0x80000300, OS_TV_NTSC);
    memset(__tlb(APP_BSS_ADDR), 0x00, APP_BSS_SIZE);
    sp.ll = (s32)APP_STACK;
    app_main();
    return EXIT_SUCCESS;
}
