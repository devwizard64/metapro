static void gdp_g_loadtile(u32 w0, u32 w1)
{
    struct tile *tile = &gdp_tile[w1 >> 24 & 7];
    u8 *dst = &gdp_tmem[tile->tmem];
    uint siz = tile->fmt & 3;
    uint sl = w0 >> 14 & 0x3FF;
    uint tl = w0 >>  2 & 0x3FF;
    uint sh = w1 >> 14 & 0x3FF;
    uint th = w1 >>  2 & 0x3FF;
    uint t;
    uint s;
    for (t = tl; t <= th; t++)
    {
        for (s = sl; s <= sh; s++)
        {
            uint i = gdp_timg_w*t+s;
            switch (siz)
            {
                case G_IM_SIZ_4b:
                    *dst++ = gdp_timg[(i)^AX_B];
                    s++;
                    break;
                case G_IM_SIZ_8b:
                    *dst++ = gdp_timg[(i)^AX_B];
                    break;
                case G_IM_SIZ_16b:
                    *dst++ = gdp_timg[(2*i+0)^AX_B];
                    *dst++ = gdp_timg[(2*i+1)^AX_B];
                    break;
                case G_IM_SIZ_32b:
                    *dst++ = gdp_timg[(4*i+0)^AX_B];
                    *dst++ = gdp_timg[(4*i+1)^AX_B];
                    *dst++ = gdp_timg[(4*i+2)^AX_B];
                    *dst++ = gdp_timg[(4*i+3)^AX_B];
                    break;
            }
        }
    }
    gsp_new_texture = true;
}
