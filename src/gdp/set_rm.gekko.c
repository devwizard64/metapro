static void gsp_set_mp(f32 mf[4][4]);

static void gdp_set_rm(u32 mode)
{
    GX_SetZMode(ZR, GX_LEQUAL, ZW);
    if (AC)
    {
        GX_SetAlphaCompare(GX_GEQUAL, 0x20, GX_AOP_AND, GX_ALWAYS, 0x00);
        GX_SetZCompLoc(GX_FALSE);
    }
    else
    {
        GX_SetAlphaCompare(GX_ALWAYS, 0x00, GX_AOP_AND, GX_ALWAYS, 0x00);
        GX_SetZCompLoc(GX_TRUE);
    }
    GX_SetBlendMode(
        BL ? GX_BM_BLEND : GX_BM_NONE, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA,
        GX_LO_NOOP
    );
    gsp_decal = DE;
    gsp_flush_mp();
}
