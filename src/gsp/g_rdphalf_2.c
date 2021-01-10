static void gsp_g_rdphalf_2(unused u32 w0, u32 w1)
{
    gsp_texrect_dsdx = ((32/4.0F)/0x0400) * (s16)(w1 >> 16);
    gsp_texrect_dtdy = ((32/4.0F)/0x0400) * (s16)(w1 >>  0);
    gsp_flush_texrect();
}
