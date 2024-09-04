static void g_rdphalf_2(UNUSED u32 w0, u32 w1)
{
	gdp_texrect[3] = w1;
	gsp_texrect();
}
