static void g_obj_loadtxtr(UNUSED u32 w0, u32 w1)
{
	uObjTxtr *txtr = gsp_addr(w1);
	switch (txtr->type)
	{
	case G_OBJLT_TLUT:
		byteswap(
			&gdp_tmem[txtr->tmem << 3], gsp_addr(txtr->image),
			(txtr->twidth+1) << 1
		);
		break;
	default:
		wdebug("unknown uObjTxtr 0x%08" FMT_X "\n", txtr->type);
		break;
	}
}
