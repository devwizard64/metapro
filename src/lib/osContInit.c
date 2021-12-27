#include "types.h"
#include "cpu.h"
#include "sys.h"

void lib_osContInit(void)
{
    PTR status = a2;
    uint bitpattern = 0;
    uint i;
    for (i = 0; i < MAXCONTROLLERS; i++)
    {
        if (os_cont_status[i].errno_ == 0) bitpattern |= 1 << i;
        *cpu_u16(status+0) = os_cont_status[i].type;
        *cpu_u8 (status+2) = os_cont_status[i].status;
        *cpu_u8 (status+3) = os_cont_status[i].errno_;
        status += 4;
    }
    *cpu_u8(a1) = bitpattern;
    v0 = 0;
}
