static void gdp_g_settile(u32 w0, u32 w1)
{
    uint tile;
    gsp_texture_fmt = w0 >> 19 & 0x1F;
    tile            = w1 >> 24 & 0x07;
    if (tile == G_TX_RENDERTILE)
    {
        gsp_texture_flag[1]  = w1 >> 18 & 0x03;
        gsp_texture_shift[1] = w1 >> 10 & 0x07;
        gsp_texture_flag[0]  = w1 >>  8 & 0x03;
        gsp_texture_shift[0] = w1 >>  0 & 0x07;
    }
    gsp_change |= CHANGE_TEXTURE;
}
