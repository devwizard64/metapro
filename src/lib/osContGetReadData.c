void lib_osContGetReadData(void)
{
    PTR pad;
    uint i;
    for (i = 0; i < 4; i++)
    {
        os_pad[i].button  = 0;
        os_pad[i].stick_x = 0;
        os_pad[i].stick_y = 0;
        os_pad[i].errno_  = CONT_NO_RESPONSE_ERROR;
    }
    input_update();
    contdemo_update();
    pad = a0;
    for (i = 0; i < 4; i++)
    {
        __write_u16(pad+0, os_pad[i].button);
        __write_u8 (pad+2, os_pad[i].stick_x);
        __write_u8 (pad+3, os_pad[i].stick_y);
        __write_u8 (pad+4, os_pad[i].errno_);
        pad += 6;
    }
}
