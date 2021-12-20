static void gdp_g_setscissor(u32 w0, u32 w1)
{
    int l = w0 >> 12 & 0xFFF;
    int t = w0 >>  0 & 0xFFF;
    int r = w1 >> 12 & 0xFFF;
    int b = w1 >>  0 & 0xFFF;
    gdp_set_sc(l, r, t, b);
}
