static void gdp_g_settimg(unused u32 w0, u32 w1)
{
    gdp_timg = gsp_addr(w1);
    gdp_timg_w = (w0 >> 0 & 0xFFF) + 1;
}
