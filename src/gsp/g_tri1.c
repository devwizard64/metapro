static void gsp_g_tri1(unused u32 w0, unused u32 w1)
{
    u8 t[3];
    gsp_flush_rect();
#ifdef GSP_F3D
    t[0] = (w1 >> 16 & 0xFF) / 10;
    t[1] = (w1 >>  8 & 0xFF) / 10;
    t[2] = (w1 >>  0 & 0xFF) / 10;
#endif
#ifdef GSP_F3DEX2
    t[0] = (w0 >> 16 & 0xFF) / 2;
    t[1] = (w0 >>  8 & 0xFF) / 2;
    t[2] = (w0 >>  0 & 0xFF) / 2;
#endif
    gsp_triangle(t);
}
