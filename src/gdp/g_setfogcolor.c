static void g_setfogcolor(UNUSED u32 w0, u32 w1)
{
    gdp_fog[0] = w1 >> 24;
    gdp_fog[1] = w1 >> 16;
    gdp_fog[2] = w1 >>  8;
    gdp_fog[3] = w1 >>  0;
    gsp_new_fog = TRUE;
}
