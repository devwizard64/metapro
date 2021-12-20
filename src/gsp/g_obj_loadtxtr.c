static void gsp_g_obj_loadtxtr(unused u32 w0, u32 w1)
{
    struct obj_txtr *txtr = gsp_addr(w1);
    switch (txtr->type)
    {
        case G_OBJLT_TLUT:
            byteswap(
                &gdp_tmem[txtr->tmem << 3], gsp_addr(txtr->image),
                (txtr->twidth+1) << 1
            );
            break;
        default:
            wdebug("unknown loadtxtr 0x%08" FMT_X "\n", txtr->type);
            break;
    }
}
