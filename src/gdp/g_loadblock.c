static void gdp_g_loadblock(unused u32 w0, unused u32 w1)
{
    struct tile *tile = &gdp_tile[w1 >> 24 & 7];
    u8 *dst = &gdp_tmem[tile->tmem];
    uint siz = tile->fmt & 3;
    uint len = w1 >> 12 & 0xFFF;
    __BYTESWAP(dst, gdp_timg, (len+1) << siz >> 1);
    gsp_new_texture = true;
}
