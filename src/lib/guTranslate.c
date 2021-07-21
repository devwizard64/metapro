void lib_guTranslate(void)
{
    f32 mf[4][4];
    mtxf_translate(mf, a1.f[IX], a2.f[IX], a3.f[IX]);
    mtx_write(__tlb(a0.i[IX]), &mf[0][0]);
}
