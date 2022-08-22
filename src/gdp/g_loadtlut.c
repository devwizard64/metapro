#ifndef APP_UNSM
static void gdp_g_loadtlut(u32 w0, u32 w1)
{
    TILE *tile = &gdp_tile[w1 >> 24 & 7];
    uint tmem = tile->tmem << 3;
    uint il = (w0 >> 14 & 0x3FF);
    uint ih = (w1 >> 14 & 0x3FF) + 1;
    uint i;
    for (i = 0; i < (ih-il) << 1; i++)
    {
        gdp_tmem[tmem++ & 0xFFF] = gdp_timg[i^AX_B];
    }
    gsp_new_texture = TRUE;
}
#else
#define gdp_g_loadtlut          NULL
#endif
