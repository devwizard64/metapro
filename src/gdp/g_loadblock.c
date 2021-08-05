static void gdp_g_loadblock(unused u32 w0, unused u32 w1)
{
    gsp_texture_len = ((w1 >> 12 & 0x0FFF)+1) << (gsp_texture_fmt & 0x03) >> 1;
    gsp_change |= CHANGE_TEXTURE;
}
