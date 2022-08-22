static void gsp_g_tri1(UNUSED u32 w0, UNUSED u32 w1)
{
    u8 t[3];
#ifdef GSP_F3D
    t[0] = (w1 >> 16 & 0xFF) / GSP_TRI_LEN;
    t[1] = (w1 >>  8 & 0xFF) / GSP_TRI_LEN;
    t[2] = (w1 >>  0 & 0xFF) / GSP_TRI_LEN;
#endif
#ifdef GSP_F3DEX2
    t[0] = (w0 >> 16 & 0xFF) / GSP_TRI_LEN;
    t[1] = (w0 >>  8 & 0xFF) / GSP_TRI_LEN;
    t[2] = (w0 >>  0 & 0xFF) / GSP_TRI_LEN;
#endif
    gsp_flush_rect();
    gdp_triangle(t);
}
