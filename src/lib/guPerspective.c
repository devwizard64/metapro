void lib_guPerspective(void)
{
    f32 mtxf[4][4];
    f32 y = a2.f[IX];
#ifdef LIB_DYNRES
    f32 a = (f32)lib_video_w / (f32)lib_video_h;
#else
    f32 a = a3.f[IX];
#endif
    f32 n = __read_f32(sp.i[IX] + 0x0010);
    f32 f = __read_f32(sp.i[IX] + 0x0014);
    mtxf_perspective(mtxf, y, a, n, f);
    mtx_write(__tlb(a0.i[IX]), &mtxf[0][0]);
}
