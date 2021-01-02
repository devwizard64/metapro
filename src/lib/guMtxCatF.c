void lib_guMtxCatF(void)
{
    f32 dst[4][4];
    f32 a[4][4];
    f32 b[4][4];
    memcpy(a, __tlb(a0.i[IX]), sizeof(a));
    memcpy(b, __tlb(a1.i[IX]), sizeof(b));
    mtxf_mul(dst, a, b);
    memcpy(__tlb(a2.i[IX]), dst, sizeof(dst));
}
