static void gdp_g_setfillcolor(unused u32 w0, u32 w1)
{
    gdp_fill = w1 >> 16;
}
