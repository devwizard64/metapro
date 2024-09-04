#include "types.h"
#include "cpu.h"
#include "sys.h"

void lib_osContGetQuery(void)
{
	int i;
	PTR status = a0;
	for (i = 0; i < MAXCONTROLLERS; i++)
	{
		*cpu_u16(status+0) = os_cont_status[i].type;
		*cpu_u8 (status+2) = os_cont_status[i].status;
		*cpu_u8 (status+3) = os_cont_status[i].errno_;
		status += 4;
	}
}
