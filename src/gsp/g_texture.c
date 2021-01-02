static void gsp_g_texture(u32 w0, u32 w1)
{
    gsp_texture_enabled = w0 >> 0;
    if (gsp_texture_enabled)
    {
        gsp_texture_vscale[0] = (1.0F/0x10000) * (w1 >> 16 & 0xFFFF);
        gsp_texture_vscale[1] = (1.0F/0x10000) * (w1 >>  0 & 0xFFFF);
    }
    gsp_change |= CHANGE_TEXTURE_ENABLED;
}
