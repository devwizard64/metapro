#ifndef APP_UNSM
static void gdp_g_loadtlut(u32 w0, u32 w1)
{
    struct tile *tile = &gdp_tile[w1 >> 24 & 7];
    u8 *dst = &gdp_tmem[tile->tmem];
    uint il = w0 >> 14 & 0x3FF;
    uint ih = w1 >> 14 & 0x3FF;
    uint i;
    for (i = il; i <= ih; i++)
    {
        *dst++ = gdp_timg[(2*i+0)^AX_B];
        *dst++ = gdp_timg[(2*i+1)^AX_B];
    }
    gsp_new_texture = true;
}
#else
#define gdp_g_loadtlut          NULL
#endif
