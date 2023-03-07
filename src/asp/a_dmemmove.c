static void a_dmemmove(u32 w0, u32 w1)
{
    uint count = w1 & 0xFFFF;
    if (count > 0)
    {
        s16 *dmemin  = asp_s16(asp_sample + (w0 & 0xFFFF));
        s16 *dmemout = asp_s16(asp_sample + (w1 >> 16   ));
        memmove(dmemout, dmemin, (count+15) & ~15);
    }
}
