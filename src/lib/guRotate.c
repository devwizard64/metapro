void lib_guRotate(void)
{
    f32 mtxf[4][4];
    mtxf_rotate(
        mtxf, a1.f[IX], a2.f[IX], a3.f[IX], __read_f32(sp.i[IX] + 0x0010)
    );
    mtx_write(__tlb(a0.i[IX]), &mtxf[0][0]);
}
