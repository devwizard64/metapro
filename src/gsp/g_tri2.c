static void g_tri2(u32 w0, u32 w1)
{
	u8 a[3];
	u8 b[3];
	a[0] = (w0 >> 16 & 0xFF) / GSP_TRI_LEN;
	a[1] = (w0 >>  8 & 0xFF) / GSP_TRI_LEN;
	a[2] = (w0 >>  0 & 0xFF) / GSP_TRI_LEN;
	b[0] = (w1 >> 16 & 0xFF) / GSP_TRI_LEN;
	b[1] = (w1 >>  8 & 0xFF) / GSP_TRI_LEN;
	b[2] = (w1 >>  0 & 0xFF) / GSP_TRI_LEN;
	gsp_flush_rect();
	gdp_triangle(a);
	gdp_triangle(b);
}
