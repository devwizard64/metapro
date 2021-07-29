void lib_guLookAtF(void)
{
    mtxf_lookat(
        __tlb(a0.i[IX]), a1.f[IX], a2.f[IX], a3.f[IX],
        __read_f32(sp.i[IX] + 0x0010),
        __read_f32(sp.i[IX] + 0x0014),
        __read_f32(sp.i[IX] + 0x0018),
        __read_f32(sp.i[IX] + 0x001C),
        __read_f32(sp.i[IX] + 0x0020),
        __read_f32(sp.i[IX] + 0x0024)
    );
}
