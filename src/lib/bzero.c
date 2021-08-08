void lib_bzero(void)
{
#ifdef __EB__
    memset(__tlb(a0), 0x00, a1);
#else
    while (a1-- != 0)
    {
        __write_u8(a0++, 0x00);
    }
#endif
}
