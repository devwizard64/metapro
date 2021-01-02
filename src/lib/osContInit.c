void lib_osContInit(void)
{
    __write_u8(a1.i[IX], 0x0F);
    v0.ll = (s32)0;
}
