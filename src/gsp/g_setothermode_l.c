static void gsp_g_setothermode_l(u32 w0, u32 w1)
{
    uint shift = w0 >> 8 & 0xFF;
    uint mask  = w0 >> 0 & 0xFF;
    u32  clear;
#ifdef GSP_F3D
    clear = ((1 << mask)-1) << shift;
#endif
#ifdef GSP_F3DEX2
    clear = (u32)((s32)(1 << 31) >> mask) >> shift;
#endif
    gdp_othermode_l &= ~clear;
    gdp_othermode_l |= w1;
    if (clear & (0x1FFFFFFF << G_MDSFT_RENDERMODE | 3 << G_MDSFT_ALPHACOMPARE))
    {
        gsp_change |= CHANGE_RENDERMODE;
    }
}
