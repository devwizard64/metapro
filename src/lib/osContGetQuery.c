void lib_osContGetQuery(void)
{
#ifdef APP_UNSM
    os_cont_status(a0, 0, CONT_TYPE_NORMAL, 0, 0);
    os_cont_status(a0, 1, CONT_TYPE_NORMAL, 0, 0);
    os_cont_status(a0, 2, CONT_TYPE_NORMAL, 0, 0);
    os_cont_status(a0, 3, CONT_TYPE_NORMAL, 0, 0);
#else
    os_cont_status(a0, 0, CONT_TYPE_NORMAL, 0, 0);
    os_cont_status(a0, 1, 0, 0, CONT_NO_RESPONSE_ERROR);
    os_cont_status(a0, 2, 0, 0, CONT_NO_RESPONSE_ERROR);
    os_cont_status(a0, 3, 0, 0, CONT_NO_RESPONSE_ERROR);
#endif
}
