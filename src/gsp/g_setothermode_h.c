static GDP_COMBINE gdp_combine_cc_shade;
static GDP_COMBINE gdp_combine_ac_shade;

static void gsp_g_setothermode_h(u32 w0, u32 w1)
{
    uint shift = w0 >> 8 & 0xFF;
    uint mask  = w0 >> 0 & 0xFF;
    u32  filter;
#ifdef GSP_F3DEX2
    mask++;
    shift = 32-shift-mask;
#endif
    gdp_othermode_h &= ~(((1 << mask) - 1) << shift);
    gdp_othermode_h |= w1;
    gdp_cycle = (gdp_othermode_h >> G_MDSFT_CYCLETYPE) & 2;
    filter = gdp_othermode_h & (0x03 << G_MDSFT_TEXTFILT);
    gdp_texture_filter =
        filter == G_TF_POINT || gdp_cycle ? GL_NEAREST : GL_LINEAR;
    if (shift == G_MDSFT_CYCLETYPE)
    {
        gsp_change |= CHANGE_RENDERMODE;
        if (gdp_cycle)
        {
            gdp_combine_cc = gdp_combine_cc_shade;
            gdp_combine_ac = gdp_combine_ac_shade;
        }
    }
}
