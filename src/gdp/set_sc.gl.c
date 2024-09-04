static void gdp_set_sc(int l, int r, int t, int b)
{
	glScissor(
		(    l) * video_w/1280,
		(960-b) * video_h/ 960,
		(  r-l) * video_w/1280,
		(  b-t) * video_h/ 960
	);
}
