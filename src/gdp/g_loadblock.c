static void gdp_g_loadblock(unused u32 w0, unused u32 w1)
{
    TILE *tile = &gdp_tile[w1 >> 24 & 7];
    u8 *dst = &gdp_tmem[tile->tmem << 3];
    uint siz = tile->fmt & 3;
    uint len = w1 >> 12 & 0xFFF;
    uint size = (len+1) << siz;
    byteswap(dst, gdp_timg, size >> 1);
    gdp_texture_size[0] = 0;
    gdp_texture_size[1] = size;
    gsp_new_texture = true;
}
