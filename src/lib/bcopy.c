void lib_bcopy(void)
{
#ifdef __EB__
    memmove(__tlb(a1), __tlb(a0), a2);
#else
    while (a2-- != 0) __write_u8(a1++, __read_s8(a0++));
#endif
}
