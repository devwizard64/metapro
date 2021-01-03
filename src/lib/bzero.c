void lib_bzero(void)
{
#ifdef _EB
    memset(__tlb(a0.i[IX]), 0x00, a1.i[IX]);
#else
    while (a1.i[IX]-- != 0)
    {
        __write_u8(a0.i[IX]++, 0x00);
    }
#endif
}
