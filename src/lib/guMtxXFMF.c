void lib_guMtxXFMF(void)
{
    f32 (*mf)[4][4] = __tlb(a0.i[IX]);
    f32 x = a1.f[0^IX];
    f32 y = a2.f[0^IX];
    f32 z = a3.f[0^IX];
    __write_f32(__read_s32((s16)0x0010 + sp.i[IX]), MDOT4(*mf, 0));
    __write_f32(__read_s32((s16)0x0014 + sp.i[IX]), MDOT4(*mf, 1));
    __write_f32(__read_s32((s16)0x0018 + sp.i[IX]), MDOT4(*mf, 2));
}
