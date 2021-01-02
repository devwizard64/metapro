void lib_guOrtho(void)
{
    f32 mtxf[4][4];
    f32 l = a1.f[IX];
    f32 r = a2.f[IX];
    f32 b = a3.f[IX];
    f32 t = __read_f32(sp.i[IX] + 0x0010);
    f32 n = __read_f32(sp.i[IX] + 0x0014);
    f32 f = __read_f32(sp.i[IX] + 0x0018);
#ifdef LIB_DYNRES
    if (n == 0.0F && f == 3.0F)
    {
        /* scale y */
        f32 y = (1.0F/2.0F) * (t+b);
        f32 h = (1.0F/2.0F) * (t-b) * (4.0F/3.0F) * lib_video_h/lib_video_w;
        mtxf_ortho(mtxf, l, r, y-h, y+h, n, f);
    }
    else
    {
        /* scale x */
        f32 x = (1.0F/2.0F) * (r+l);
        f32 w = (1.0F/2.0F) * (r-l) * (3.0F/4.0F) * lib_video_w/lib_video_h;
        mtxf_ortho(mtxf, x-w, x+w, b, t, n, f);
    }
#else
    mtxf_ortho(mtxf, l, r, b, t, n, f);
#endif
    mtx_write(__tlb(a0.i[IX]), &mtxf[0][0]);
}
