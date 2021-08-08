void lib_guOrtho(void)
{
    f32 mf[4][4];
    f32 l = ARG_F(a1);
    f32 r = ARG_F(a2);
    f32 b = ARG_F(a3);
    f32 t = __read_f32(sp + 0x0010);
    f32 n = __read_f32(sp + 0x0014);
    f32 f = __read_f32(sp + 0x0018);
#ifdef LIB_DYNRES
    if (n == 0 && f == 3)
    {
        /* scale y */
        f32 y =             (1.0F/2) * (t+b);
        f32 h = (4.0F/3.0F)*(1.0F/2) * (t-b) * lib_video_h/lib_video_w;
        mtxf_ortho(mf, l, r, y-h, y+h, n, f);
    }
    else
    {
        /* scale x */
        f32 x =             (1.0F/2) * (r+l);
        f32 w = (3.0F/4.0F)*(1.0F/2) * (r-l) * lib_video_w/lib_video_h;
        mtxf_ortho(mf, x-w, x+w, b, t, n, f);
    }
#else
    mtxf_ortho(mf, l, r, b, t, n, f);
#endif
    mtx_write(__tlb(a0), &mf[0][0]);
}
