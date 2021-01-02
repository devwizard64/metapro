static void gsp_g_setothermode_h(u32 w0, u32 w1)
{
    uint shift = w0 >> 8 & 0xFF;
    uint mask  = w0 >> 0 & 0xFF;
    u32  filter;
#ifdef GSP_F3DEX2
    mask++;
    shift = 32-shift-mask;
#endif
    gsp_othermode_h &= ~(((1 << mask) - 1) << shift);
    gsp_othermode_h |= w1;
    gsp_cycle = gsp_othermode_h & (0x02 << G_MDSFT_CYCLETYPE);
    filter = gsp_othermode_h & (0x03 << G_MDSFT_TEXTFILT);
    gsp_texture_filter =
        filter == G_TF_POINT || gsp_cycle ? GL_NEAREST : GL_LINEAR;
    if (shift == G_MDSFT_CYCLETYPE)
    {
        gsp_change |= CHANGE_RENDERMODE;
        if (gsp_cycle)
        {
            gsp_combine_cc = gsp_combine_cc_shade;
            gsp_combine_ac = gsp_combine_ac_shade;
        }
    }
}
