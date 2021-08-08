void lib_guMtxCatL(void)
{
    f32 mf[4][4];
    f32 a[4][4];
    f32 b[4][4];
    mtx_read(&a[0][0], __tlb(a0));
    mtx_read(&b[0][0], __tlb(a1));
    mtxf_cat(mf, a, b);
    mtx_write(__tlb(a2), &mf[0][0]);
}
