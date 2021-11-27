static void gdp_g_setcombine(u32 w0, u32 w1)
{
    gdp_combine_w0 = w0;
    gdp_combine_w1 = w1;
    gdp_flush_cc();
}
