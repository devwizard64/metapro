static void gdp_g_settile(u32 w0, u32 w1)
{
    struct tile *tile = &gdp_tile[w1 >> 24 & 7];
    tile->line      = w0 >>  9 & 0x1FF;
    tile->tmem      = w0 >>  0 & 0x1FF;
    tile->pal       = w1 >> 20 & 0x0F;
    tile->fmt       = w0 >> 19 & 0x1F;
    tile->cm[0]     = w1 >>  8 & 3;
    tile->cm[1]     = w1 >> 18 & 3;
    tile->shift[0]  = w1 >>  0 & 7;
    tile->shift[1]  = w1 >> 10 & 7;
}
