static void gsp_g_rdphalf_2(unused u32 w0, u32 w1)
{
    gdp_texrect[3] = w1;
    gsp_flush_texrect();
}
