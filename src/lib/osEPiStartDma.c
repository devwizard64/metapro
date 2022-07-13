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
    int   direction = a2;
    devAddr &= 0x0FFFFFFF;
#ifdef SRAM
    if (devAddr >= 0x08000000)
    {
        switch (direction)
        {
            case OS_READ:   sram_read(dramAddr, devAddr, size);     break;
            case OS_WRITE:  sram_write(devAddr, dramAddr, size);    break;
        }
    }
    else
#endif
    {
        switch (direction)
        {
            case OS_READ:   dma(dramAddr, devAddr, size);   break;
        }
    }
    v0 = mesg_send(mq, 0, OS_MESG_NOBLOCK);
}
