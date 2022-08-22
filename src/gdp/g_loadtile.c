static void gdp_g_loadtile(u32 w0, u32 w1)
{
    TILE *tile = &gdp_tile[w1 >> 24 & 7];
    uint tmem = tile->tmem << 3;
    uint siz = tile->fmt & 3;
    uint t;
    uint s;
    tile->ul[0] = (w0 >> 14 & 0x3FF);
    tile->ul[1] = (w0 >>  2 & 0x3FF);
    tile->lr[0] = (w1 >> 14 & 0x3FF) + 1;
    tile->lr[1] = (w1 >>  2 & 0x3FF) + 1;
    for (t = tile->ul[1]; t < tile->lr[1]; t++)
    {
        for (s = tile->ul[0]; s < tile->lr[0]; s++)
        {
            uint i = gdp_timg_w*t+s;
            switch (siz)
            {
                case G_IM_SIZ_4b:
                    gdp_tmem[tmem++ & 0xFFF] = gdp_timg[(i >> 1 | 0)^AX_B];
                    s++;
                    break;
                case G_IM_SIZ_8b:
                    gdp_tmem[tmem++ & 0xFFF] = gdp_timg[(i << 0 | 0)^AX_B];
                    break;
                case G_IM_SIZ_16b:
                    gdp_tmem[tmem++ & 0xFFF] = gdp_timg[(i << 1 | 0)^AX_B];
                    gdp_tmem[tmem++ & 0xFFF] = gdp_timg[(i << 1 | 1)^AX_B];
                    break;
                case G_IM_SIZ_32b:
                    gdp_tmem[tmem++ & 0xFFF] = gdp_timg[(i << 2 | 0)^AX_B];
                    gdp_tmem[tmem++ & 0xFFF] = gdp_timg[(i << 2 | 1)^AX_B];
                    gdp_tmem[tmem++ & 0xFFF] = gdp_timg[(i << 2 | 2)^AX_B];
                    gdp_tmem[tmem++ & 0xFFF] = gdp_timg[(i << 2 | 3)^AX_B];
                    break;
            }
        }
    }
    gdp_texture_size[0] = tile->lr[0] - tile->ul[0];
    gdp_texture_size[1] = tile->lr[1] - tile->ul[1];
    gsp_new_texture = TRUE;
}
