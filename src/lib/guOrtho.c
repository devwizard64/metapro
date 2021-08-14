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
