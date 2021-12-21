#include "types.h"
#include "cpu.h"
#include "sys.h"

void lib_osContGetQuery(void)
{
    PTR status = a0;
    uint i;
    for (i = 0; i < MAXCONTROLLERS; i++)
    {
        *__u16(status+0) = os_cont_status[i].type;
        *__u8 (status+2) = os_cont_status[i].status;
        *__u8 (status+3) = os_cont_status[i].errno_;
        status += 4;
    }
}
