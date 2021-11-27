static GDP_COMBINE gdp_combine_cc_shade;
static GDP_COMBINE gdp_combine_ac_shade;

static void gsp_g_setothermode_h(u32 w0, u32 w1)
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
    gdp_othermode_h &= ~clear;
    gdp_othermode_h |= w1;
    gdp_cycle = gdp_othermode_h & (2 << G_MDSFT_CYCLETYPE);
    gdp_tf =
        !gdp_cycle && (gdp_othermode_h & G_TF_BILERP) ?
            GDP_TF_BILERP : GDP_TF_POINT;
    if (clear & (3 << G_MDSFT_CYCLETYPE))
    {
        gdp_rect = 0;
        gdp_flush_rm();
    }
}
