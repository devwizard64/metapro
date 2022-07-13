#include "types.h"
#include "cpu.h"
#include "sys.h"

#include "ultra64.h"

void lib_osContGetReadData(void)
{
    int i;
    PTR pad;
    for (i = 0; i < MAXCONTROLLERS; i++)
    {
        os_cont_pad[i].button  = 0;
        os_cont_pad[i].stick_x = 0;
        os_cont_pad[i].stick_y = 0;
        os_cont_pad[i].errno_  = CONT_NO_RESPONSE_ERROR;
    }
    input_update();
    contdemo_update();
    pad = a0;
    for (i = 0; i < MAXCONTROLLERS; i++)
    {
        *cpu_u16(pad+0) = os_cont_pad[i].button;
        *cpu_s8 (pad+2) = os_cont_pad[i].stick_x;
        *cpu_s8 (pad+3) = os_cont_pad[i].stick_y;
        *cpu_u8 (pad+4) = os_cont_pad[i].errno_;
        pad += 6;
    }
}
