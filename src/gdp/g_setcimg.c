static void gdp_g_setcimg(unused u32 w0, u32 w1)
{
    gdp_cimg = gsp_addr(w1);
}
