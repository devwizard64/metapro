static void gdp_g_setscissor(u32 w0, u32 w1)
{
    int l = w0 >> 12 & 0x0FFF;
    int t = w0 >>  0 & 0x0FFF;
    int r = w1 >> 12 & 0x0FFF;
    int b = w1 >>  0 & 0x0FFF;
#ifdef GEKKO
    GX_SetScissor(
        (  l) * lib_video_w/1280,
        (  t) * lib_video_h/ 960,
        (r-l) * lib_video_w/1280,
        (b-t) * lib_video_h/ 960
    );
#else
    glScissor(
        (    l) * lib_video_w/1280,
        (960-b) * lib_video_h/ 960,
        (  r-l) * lib_video_w/1280,
        (  b-t) * lib_video_h/ 960
    );
#endif
}
