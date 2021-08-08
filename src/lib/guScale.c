void lib_guScale(void)
{
    f32 mf[4][4];
    mtxf_scale(mf, ARG_F(a1), ARG_F(a2), ARG_F(a3));
    mtx_write(__tlb(a0), &mf[0][0]);
}
