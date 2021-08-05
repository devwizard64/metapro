static void gdp_g_setprimcolor(unused u32 w0, u32 w1)
{
    gdp_prim[0] = w1 >> 24;
    gdp_prim[1] = w1 >> 16;
    gdp_prim[2] = w1 >>  8;
    gdp_prim[3] = w1 >>  0;
}
