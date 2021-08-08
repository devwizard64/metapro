#define pad_set(i, button, x, y, code)  \
{                                       \
    __write_u16(pad+6*i+0, button);     \
    __write_u8( pad+6*i+2, x);          \
    __write_u8( pad+6*i+3, y);          \
    __write_u8( pad+6*i+4, code);       \
}

void lib_osContGetReadData(void)
{
    PTR pad;
    input_update();
    pad = a0;
    pad_set(0, lib_pad.button, lib_pad.stick_x, lib_pad.stick_y, 1);
#ifdef APP_UNSM
    pad_set(1, lib_pad.button, lib_pad.stick_x, lib_pad.stick_y, 1);
    pad_set(2, lib_pad.button, lib_pad.stick_x, lib_pad.stick_y, 1);
    pad_set(3, lib_pad.button, lib_pad.stick_x, lib_pad.stick_y, 1);
#else
    pad_set(1, 0, 0, 0, 0);
    pad_set(2, 0, 0, 0, 0);
    pad_set(3, 0, 0, 0, 0);
#endif
}
