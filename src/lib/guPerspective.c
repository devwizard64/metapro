void lib_guPerspective(void)
{
    f32 mf[4][4];
    f32 y = ARG_F(a2);
#ifdef LIB_DYNRES
    f32 a = (f32)lib_video_w / (f32)lib_video_h;
#else
    f32 a = ARG_F(a3);
#endif
    f32 n = __read_f32(sp + 0x0010);
    f32 f = __read_f32(sp + 0x0014);
    mtxf_perspective(mf, y, a, n, f);
    mtx_write(__tlb(a0), &mf[0][0]);
}
