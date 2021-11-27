void lib_guPerspective(void)
{
    f32 mf[4][4];
    float y = ARG_F(a2);
#ifdef LIB_DYNRES
    float a = lib_video_aspect;
#else
    float a = ARG_F(a3);
#endif
    float n = __read_f32(sp + 0x0010);
    float f = __read_f32(sp + 0x0014);
    mtxf_perspective(mf, y, a, n, f);
    mtx_write(__tlb(a0), &mf[0][0]);
}
