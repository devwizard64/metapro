static void asp_a_loadbuff(unused u32 w0, u32 w1)
{
    if (asp_count > 0)
    {
        __WORDSWAP(asp_s16(asp_dmemin), asp_addr(w1), asp_count);
    }
}
