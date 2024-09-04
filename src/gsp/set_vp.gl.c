static void gsp_set_vp(int l, int r, int t, int b)
{
	glViewport(
		(    l) * video_w/1280,
		(960-b) * video_h/ 960,
		(  r-l) * video_w/1280,
		(  b-t) * video_h/ 960
	);
}
