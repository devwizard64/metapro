#include "types.h"
#include "cpu.h"
#include "app.h"

void lib_osInitialize(void)
{
    memcpy(__dram(0x80000000), __dram(__osExceptionPreamble), 0x00000010);
    memcpy(__dram(0x80000080), __dram(__osExceptionPreamble), 0x00000010);
    memcpy(__dram(0x80000100), __dram(__osExceptionPreamble), 0x00000010);
    memcpy(__dram(0x80000180), __dram(__osExceptionPreamble), 0x00000010);
}
