void lib_guRotate(void)
{
    f32 mf[4][4];
    mtxf_rotate(
        mf, a1.f[IX], a2.f[IX], a3.f[IX], __read_f32(sp.i[IX] + 0x0010)
    );
    mtx_write(__tlb(a0.i[IX]), &mf[0][0]);
}
