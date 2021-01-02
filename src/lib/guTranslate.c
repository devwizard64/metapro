void lib_guTranslate(void)
{
    f32 mtxf[4][4];
    mtxf_translate(mtxf, a1.f[IX], a2.f[IX], a3.f[IX]);
    mtx_write(__tlb(a0.i[IX]), &mtxf[0][0]);
}
