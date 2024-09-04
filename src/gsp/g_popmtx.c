static void g_popmtx(UNUSED u32 w0, UNUSED u32 w1)
{
#ifdef GSP_F3D
	gsp_mtx_modelview--;
#endif
#ifdef GSP_F3DEX2
	gsp_mtx_modelview -= w1 / 0x40;
#endif
	gsp_new_light = TRUE;
	gsp_flush_mm();
}
