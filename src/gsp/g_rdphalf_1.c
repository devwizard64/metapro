static void gsp_g_rdphalf_1(unused u32 w0, u32 w1)
{
#ifdef GSP_F3DEX2
    gsp_rdphalf_1 = w1;
#endif
    gsp_texrect_ul = (1.0F/32) * (s16)(w1 >> 16);
    gsp_texrect_vl = (1.0F/32) * (s16)(w1 >>  0);
}
