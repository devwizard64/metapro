#ifdef APP_UNSM
#define __osExceptionPreamble   0x80327640
#endif
#ifdef APP_UNK4
#define __osExceptionPreamble   0x8002DF60
#endif
void lib_osInitialize(void)
{
    memcpy(__tlb(0x80000000), __tlb(__osExceptionPreamble), 0x00000010);
    memcpy(__tlb(0x80000080), __tlb(__osExceptionPreamble), 0x00000010);
    memcpy(__tlb(0x80000100), __tlb(__osExceptionPreamble), 0x00000010);
    memcpy(__tlb(0x80000180), __tlb(__osExceptionPreamble), 0x00000010);
}
