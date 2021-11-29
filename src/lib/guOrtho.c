void lib_guOrtho(void)
{
    f32 mf[4][4];
    float l = ARG_F(a1);
    float r = ARG_F(a2);
    float b = ARG_F(a3);
    float t = __read_f32(sp + 0x0010);
    float n = __read_f32(sp + 0x0014);
    float f = __read_f32(sp + 0x0018);
#ifdef LIB_DYNRES
    if (n == 0 && f == 3)
    {
    #ifdef __NDS__
        n = -3;
        f = 0;
    #endif
        mtxf_ortho_bg(mf, l, r, b, t, n, f);
    }
    else
    {
        mtxf_ortho_fg(mf, l, r, b, t, n, f);
    }
#else
    mtxf_ortho(mf, l, r, b, t, n, f);
#endif
    mtx_write(__tlb(a0), &mf[0][0]);
}
