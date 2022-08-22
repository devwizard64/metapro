static void gdp_g_setcimg(UNUSED u32 w0, u32 w1)
{
    gdp_cimg = gsp_addr(w1);
}
