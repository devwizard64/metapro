static void gdp_g_settimg(unused u32 w0, u32 w1)
{
    gsp_texture_addr = gsp_addr(w1);
    gsp_change |= CHANGE_TEXTURE;
}
