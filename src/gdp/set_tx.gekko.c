static void gdp_set_tx(GDP_TX tx)
{
    GX_SetTevOrder(
        GX_TEVSTAGE0,
        tx != GDP_TX_NULL ? GX_TEXCOORD0 : GX_TEXCOORDNULL,
        tx != GDP_TX_NULL ? GX_TEXMAP0   : GX_TEXMAP_NULL,
        GX_COLOR0A0
    );
    GX_SetTevOp(GX_TEVSTAGE0, tx);
}
