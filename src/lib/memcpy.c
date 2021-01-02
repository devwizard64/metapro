void lib_memcpy(void)
{
#ifdef _GCN
    memmove(__tlb(a0.i[IX]), __tlb(a1.i[IX]), a2.i[IX]);
#else
    while (a2.i[IX]-- != 0)
    {
        __write_u8(a1.i[IX]++, __read_u8(a0.i[IX]++));
    }
#endif
}
