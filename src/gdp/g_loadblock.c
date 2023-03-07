static void g_loadblock(UNUSED u32 w0, u32 w1)
{
    TILE *tile = &gdp_tile[w1 >> 24 & 7];
    uint tmem = tile->tmem << 3;
    uint siz = tile->fmt & 3;
    uint len = (w1 >> 12 & 0xFFF) + 1;
    uint i;
    for (i = 0; i < len << siz >> 1; i++)
    {
        gdp_tmem[tmem++ & 0xFFF] = gdp_timg[i^AX_B];
    }
    gdp_texture_size[0] = 0;
    gdp_texture_size[1] = len << siz;
    gsp_new_texture = TRUE;
}
