void lib_osContInit(void)
{
#ifdef APP_UNSM
    __write_u8(a1, 0x03);
    os_cont_status(a2, 0, CONT_TYPE_NORMAL, 0, 0);
    os_cont_status(a2, 1, CONT_TYPE_NORMAL, 0, 0);
    os_cont_status(a2, 2, 0, 0, CONT_NO_RESPONSE_ERROR);
    os_cont_status(a2, 3, 0, 0, CONT_NO_RESPONSE_ERROR);
#else
    __write_u8(a1, 0x01);
    os_cont_status(a2, 0, CONT_TYPE_NORMAL, 0, 0);
    os_cont_status(a2, 1, 0, 0, CONT_NO_RESPONSE_ERROR);
    os_cont_status(a2, 2, 0, 0, CONT_NO_RESPONSE_ERROR);
    os_cont_status(a2, 3, 0, 0, CONT_NO_RESPONSE_ERROR);
#endif
    v0 = 0;
}
