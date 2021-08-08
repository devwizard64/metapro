void lib_osContGetReadData(void)
{
    input_update();
    os_cont_pad(a0, 0, lib_pad.button, lib_pad.stick_x, lib_pad.stick_y, 0);
#ifdef APP_UNSM
    os_cont_pad(a0, 1, lib_pad.button, lib_pad.stick_x, lib_pad.stick_y, 0);
    os_cont_pad(a0, 2, lib_pad.button, lib_pad.stick_x, lib_pad.stick_y, 0);
    os_cont_pad(a0, 3, lib_pad.button, lib_pad.stick_x, lib_pad.stick_y, 0);
#else
    os_cont_pad(a0, 1, 0, 0, 0, CONT_NO_RESPONSE_ERROR);
    os_cont_pad(a0, 2, 0, 0, 0, CONT_NO_RESPONSE_ERROR);
    os_cont_pad(a0, 3, 0, 0, 0, CONT_NO_RESPONSE_ERROR);
#endif
}
