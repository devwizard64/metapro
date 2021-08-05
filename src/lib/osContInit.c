void lib_osContInit(void)
{
#ifdef APP_UNSM
    __write_u8(a1.i[IX], 0x0F);
#else
    __write_u8(a1.i[IX], 0x01);
#endif
    v0.ll = (s32)0;
}
