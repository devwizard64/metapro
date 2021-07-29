void lib_guLookAt(void)
{
    f32 mf[4][4];
    mtxf_lookat(
        mf, a1.f[IX], a2.f[IX], a3.f[IX],
        __read_f32(sp.i[IX] + 0x0010),
        __read_f32(sp.i[IX] + 0x0014),
        __read_f32(sp.i[IX] + 0x0018),
        __read_f32(sp.i[IX] + 0x001C),
        __read_f32(sp.i[IX] + 0x0020),
        __read_f32(sp.i[IX] + 0x0024)
    );
    mtx_write(__tlb(a0.i[IX]), &mf[0][0]);
}
