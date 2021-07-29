static void gsp_g_tri2(u32 w0, u32 w1)
{
    u8 t[3];
    gsp_flush_rect();
    t[0] = (w0 >> 16 & 0xFF) / GSP_TRI_LEN;
    t[1] = (w0 >>  8 & 0xFF) / GSP_TRI_LEN;
    t[2] = (w0 >>  0 & 0xFF) / GSP_TRI_LEN;
    gsp_triangle(t);
    t[0] = (w1 >> 16 & 0xFF) / GSP_TRI_LEN;
    t[1] = (w1 >>  8 & 0xFF) / GSP_TRI_LEN;
    t[2] = (w1 >>  0 & 0xFF) / GSP_TRI_LEN;
    gsp_triangle(t);
}
