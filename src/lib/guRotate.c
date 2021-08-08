void lib_guRotate(void)
{
    f32 mf[4][4];
    mtxf_rotate(
        mf, ARG_F(a1), ARG_F(a2), ARG_F(a3), __read_f32(sp + 0x0010)
    );
    mtx_write(__tlb(a0), &mf[0][0]);
}
