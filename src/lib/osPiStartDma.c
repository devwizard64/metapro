#include "types.h"
#include "cpu.h"
#include "sys.h"

#include "ultra64.h"

void lib_osPiStartDma(void)
{
    s32   direction = a2;
    PTR   devAddr   = a3;
    void *vAddr     = __dram(*__s32(sp+0x10));
    u32   nbytes    = *__u32(sp+0x14);
    OSMesgQueue *mq = __dram(*__s32(sp+0x18));
    switch (direction)
    {
        case OS_READ:   dma(vAddr, devAddr, nbytes);    break;
    }
    v0 = mesg_send(mq, 0, OS_MESG_NOBLOCK);
}
