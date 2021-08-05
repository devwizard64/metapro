static void gdp_g_settilesize(u32 w0, u32 w1)
{
    /* ? */
    if ((w0 & 0x00FFFFFF) == 0)
    {
        gsp_texture_size[0] = (w1 >> 14 & 0x03FF) + 1;
        gsp_texture_size[1] = (w1 >>  2 & 0x03FF) + 1;
        gsp_texture_tscale[0] = (1.0F/32) / gsp_texture_size[0];
        gsp_texture_tscale[1] = (1.0F/32) / gsp_texture_size[1];
    }
    gsp_change |= CHANGE_TEXTURE;
}
