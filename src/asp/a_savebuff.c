static void asp_a_savebuff(UNUSED u32 w0, u32 w1)
{
    if (asp_count > 0) wordswap(asp_addr(w1), asp_s16(asp_dmemout), asp_count);
}
