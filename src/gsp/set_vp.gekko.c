static void gsp_set_vp(int l, int r, int t, int b)
{
	GX_SetViewport(
		(  l) * video_w/1280.0F,
		(  t) * video_h/ 960.0F,
		(r-l) * video_w/1280.0F,
		(b-t) * video_h/ 960.0F,
		0, 1
	);
}
