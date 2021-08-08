void lib_guLookAtF(void)
{
    mtxf_lookat(
        __tlb(a0), ARG_F(a1), ARG_F(a2), ARG_F(a3),
        __read_f32(sp + 0x0010),
        __read_f32(sp + 0x0014),
        __read_f32(sp + 0x0018),
        __read_f32(sp + 0x001C),
        __read_f32(sp + 0x0020),
        __read_f32(sp + 0x0024)
    );
}
