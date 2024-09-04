#include "types.h"
#include "cpu.h"
#include "sys.h"

#include "ultra64.h"

void lib_osPiStartDma(void)
{
	int   direction = a2;
	PTR   devAddr   = a3;
	void *vAddr     = cpu_ptr(*cpu_s32(sp+0x10));
	u32   nbytes    =        (*cpu_u32(sp+0x14));
	OSMesgQueue *mq = cpu_ptr(*cpu_s32(sp+0x18));
	switch (direction)
	{
	case OS_READ:   cart_rd(vAddr, devAddr, nbytes);    break;
	}
	v0 = mesg_send(mq, 0, OS_MESG_NOBLOCK);
}
