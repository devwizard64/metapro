static void asp_a_loadadpcm(u32 w0, u32 w1)
{
    __WORDSWAP(asp_s16(asp_adpcm), asp_addr(w1), w0 >> 0 & 0xFFFF);
}
