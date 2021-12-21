#ifndef APP_UNSM
static void gdp_g_loadtlut(u32 w0, u32 w1)
{
    TILE *tile = &gdp_tile[w1 >> 24 & 7];
    u8 *dst = &gdp_tmem[tile->tmem << 3];
    uint il = w0 >> 14 & 0x3FF;
    uint ih = w1 >> 14 & 0x3FF;
    byteswap(dst, gdp_timg, (ih-il+1) << 1);
    gsp_new_texture = true;
}
#else
#define gdp_g_loadtlut          NULL
#endif
