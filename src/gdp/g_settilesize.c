static void gdp_g_settilesize(u32 w0, u32 w1)
{
    struct tile *tile = &gdp_tile[w1 >> 24 & 7];
    tile->ul[0] = (w0 >> 14 & 0x3FF);
    tile->ul[1] = (w0 >>  2 & 0x3FF);
    tile->lr[0] = (w1 >> 14 & 0x3FF) + 1;
    tile->lr[1] = (w1 >>  2 & 0x3FF) + 1;
}
