static void g_setblendcolor(UNUSED u32 w0, UNUSED u32 w1)
{
#if 0
    gdp_blend[0] = w1 >> 24;
    gdp_blend[1] = w1 >> 16;
    gdp_blend[2] = w1 >>  8;
    gdp_blend[3] = w1 >>  0;
#endif
}
