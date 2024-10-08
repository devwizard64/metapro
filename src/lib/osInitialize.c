#include "types.h"
#include "cpu.h"
#include "app.h"

void lib_osInitialize(void)
{
	void *src = cpu_ptr(__osExceptionPreamble);
	memcpy(&cpu_dram[0x000], src, 16);
	memcpy(&cpu_dram[0x080], src, 16);
	memcpy(&cpu_dram[0x100], src, 16);
	memcpy(&cpu_dram[0x180], src, 16);
}
