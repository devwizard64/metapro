static void g_fillrect(u32 w0, u32 w1)
{
	if (gdp_fill != 0xFFFC) gsp_fillrect(w0, w1);
}
