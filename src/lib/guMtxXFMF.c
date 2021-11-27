void lib_guMtxXFMF(void)
{
    f32 (*mf)[4][4] = __tlb(a0);
    float x = ARG_F(a1);
    float y = ARG_F(a2);
    float z = ARG_F(a3);
    __write_f32(__read_s32((s16)0x0010 + sp), MDOT4(*mf, 0));
    __write_f32(__read_s32((s16)0x0014 + sp), MDOT4(*mf, 1));
    __write_f32(__read_s32((s16)0x0018 + sp), MDOT4(*mf, 2));
}
