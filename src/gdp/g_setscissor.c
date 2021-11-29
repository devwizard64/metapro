static void gdp_g_setscissor(u32 w0, u32 w1)
{
    int l = w0 >> 12 & 0x0FFF;
    int t = w0 >>  0 & 0x0FFF;
    int r = w1 >> 12 & 0x0FFF;
    int b = w1 >>  0 & 0x0FFF;
    gdp_set_sc(l, r, t, b);
}
