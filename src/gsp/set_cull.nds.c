static void gsp_set_cull(u32 mode)
{
	gsp_polyfmt &= ~(3 << 6);
	if (!(mode & G_CULL_FRONT)) gsp_polyfmt |= 2 << 6;
	if (!(mode & G_CULL_BACK))  gsp_polyfmt |= 1 << 6;
}
