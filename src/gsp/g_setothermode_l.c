static void gsp_g_setothermode_l(u32 w0, u32 w1)
{
    uint shift = w0 >> 8 & 0xFF;
    uint mask  = w0 >> 0 & 0xFF;
#ifdef GSP_F3DEX2
    mask++;
    shift = 32-shift-mask;
#endif
    gdp_othermode_l &= ~(((1 << mask) - 1) << shift);
    gdp_othermode_l |= w1;
    if (shift == G_MDSFT_RENDERMODE)
    {
        gsp_change |= CHANGE_RENDERMODE;
    }
}
