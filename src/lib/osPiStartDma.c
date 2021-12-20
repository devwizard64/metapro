#include "types.h"
#include "cpu.h"
#include "sys.h"

#include "ultra64.h"

void lib_osPiStartDma(void)
{
    s32 direction = a2;
    PTR src = a3;
    void *dst = __dram(*__s32(sp+0x10));
    u32 size = *__u32(sp+0x14);
    struct os_mesg_queue *mq = __dram(*__s32(sp+0x18));
    switch (direction)
    {
        case OS_READ:   dma(dst, src, size);    break;
    }
    v0 = mesg_send(mq, 0, OS_MESG_NOBLOCK);
}
