void lib_alSeqFileNew(void)
{
    s32 *addr = __tlb(a0);
    uint len = __read_s16(a0 + 0x0002);
    do
    {
        addr[1] += a1;
        addr += 2;
        len  -= 1;
    }
    while (len > 0);
}
