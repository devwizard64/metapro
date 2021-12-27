#include "types.h"
#include "cpu.h"
#include "sys.h"

#include "ultra64.h"

void lib_osEPiStartDma(void)
{
    OSMesgQueue *mq = cpu_ptr(*cpu_s32(a1+0x04));
    void *dramAddr  = cpu_ptr(*cpu_s32(a1+0x08));
    PTR   devAddr   =        (*cpu_s32(a1+0x0C));
    u32   size      =        (*cpu_u32(a1+0x10));
    s32   direction = a2;
    devAddr &= 0x0FFFFFFF;
    if (devAddr < 0x08000000)
    {
        switch (direction)
        {
            case OS_READ:   dma(dramAddr, devAddr, size);   break;
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
