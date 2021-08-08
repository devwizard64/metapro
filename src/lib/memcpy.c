void lib_memcpy(void)
{
#ifdef __EB__
    memmove(__tlb(a0), __tlb(a1), a2);
#else
    while (a2-- != 0)
    {
        __write_u8(a1++, __read_u8(a0++));
    }
#endif
}
