static void gsp_g_texture(unused u32 w0, u32 w1)
{
    gsp_texture_scale[0] = w1 >> 16;
    gsp_texture_scale[1] = w1 >>  0;
}
