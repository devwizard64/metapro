void lib_alSeqFileNew(void)
{
    s32 *addr = __tlb(a0.i[IX]);
    uint len = __read_s16(a0.i[IX] + 0x0002);
    do
    {
        addr[1] += a1.i[IX];
        addr += 2;
        len  -= 1;
    }
    while (len > 0);
}
