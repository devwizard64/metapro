static void gdp_set_sc(int l, int r, int t, int b)
{
	GX_SetScissor(
		(  l) * video_w/1280,
		(  t) * video_h/ 960,
		(r-l) * video_w/1280,
		(b-t) * video_h/ 960
	);
}
