void lib_osContGetQuery(void)
{
    __write_u16(a0+0x00, CONT_TYPE_NORMAL);
    __write_u8( a0+0x02, 1);
    __write_u8( a0+0x03, 0);
    __write_u16(a0+0x04, CONT_TYPE_NORMAL);
    __write_u8( a0+0x06, 1);
    __write_u8( a0+0x07, 0);
    __write_u16(a0+0x08, CONT_TYPE_NORMAL);
    __write_u8( a0+0x0A, 1);
    __write_u8( a0+0x0B, 0);
    __write_u16(a0+0x0C, CONT_TYPE_NORMAL);
    __write_u8( a0+0x0E, 1);
    __write_u8( a0+0x0F, 0);
}
