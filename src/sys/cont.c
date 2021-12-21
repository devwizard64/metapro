OSContStatus os_cont_status[MAXCONTROLLERS] =
{
    {CONT_TYPE_NORMAL, 0,                      0},
#ifdef APP_UNSM
    {CONT_TYPE_NORMAL, 0,                      0},
#else
    {               0, 0, CONT_NO_RESPONSE_ERROR},
#endif
    {               0, 0, CONT_NO_RESPONSE_ERROR},
    {               0, 0, CONT_NO_RESPONSE_ERROR},
};
OSContPad os_cont_pad[MAXCONTROLLERS];
