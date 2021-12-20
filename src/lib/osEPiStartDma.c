#include "types.h"
#include "cpu.h"
#include "sys.h"

#include "ultra64.h"

void lib_osEPiStartDma(void)
{
    struct os_mesg_queue *mq = __dram(*__s32(a1+0x04));
    void *dst = __dram(*__s32(a1+0x08));
    PTR src = *__s32(a1+0x0C);
    u32 size = *__u32(a1+0x10);
    s32 direction = a2;
    src &= 0x0FFFFFFF;
    if (src < 0x08000000)
    {
        switch (direction)
        {
            case OS_READ:   dma(dst, src, size);    break;
        }
    }
    else
    {
        switch (direction)
        {
            case OS_READ:   break;
            case OS_WRITE:  break;
        }
    }
    v0 = mesg_send(mq, 0, OS_MESG_NOBLOCK);
}
