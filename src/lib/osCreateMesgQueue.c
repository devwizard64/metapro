void lib_osCreateMesgQueue(void)
{
    __write_u32(a0.i[IX] + 0x0000, 0);
    __write_u32(a0.i[IX] + 0x0004, 0);
    __write_u32(a0.i[IX] + 0x0008, 0);
    __write_u32(a0.i[IX] + 0x000C, 0);
    __write_u32(a0.i[IX] + 0x0010, a2.i[IX]);
    __write_u32(a0.i[IX] + 0x0014, a1.i[IX]);
}
