void lib_guMtxCatF(void)
{
    f32 mf[4][4];
    f32 a[4][4];
    f32 b[4][4];
    memcpy(a, __tlb(a0.i[IX]), sizeof(a));
    memcpy(b, __tlb(a1.i[IX]), sizeof(b));
    mtxf_cat(mf, a, b);
    memcpy(__tlb(a2.i[IX]), mf, sizeof(mf));
}
