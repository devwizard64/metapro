void lib_osContGetReadData(void)
{
    input_update();
    __write_u16(a0.i[IX]+0x00, lib_pad.button);
    __write_u8( a0.i[IX]+0x02, lib_pad.stick_x);
    __write_u8( a0.i[IX]+0x03, lib_pad.stick_y);
    __write_u8( a0.i[IX]+0x04, 0x01);
    __write_u16(a0.i[IX]+0x06, lib_pad.button);
    __write_u8( a0.i[IX]+0x08, lib_pad.stick_x);
    __write_u8( a0.i[IX]+0x09, lib_pad.stick_y);
    __write_u8( a0.i[IX]+0x0A, 0x01);
    __write_u16(a0.i[IX]+0x0C, lib_pad.button);
    __write_u8( a0.i[IX]+0x0E, lib_pad.stick_x);
    __write_u8( a0.i[IX]+0x0F, lib_pad.stick_y);
    __write_u8( a0.i[IX]+0x10, 0x01);
    __write_u16(a0.i[IX]+0x12, lib_pad.button);
    __write_u8( a0.i[IX]+0x14, lib_pad.stick_x);
    __write_u8( a0.i[IX]+0x15, lib_pad.stick_y);
    __write_u8( a0.i[IX]+0x16, 0x01);
}
