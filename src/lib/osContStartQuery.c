void lib_osContStartQuery(void)
{
    a1 = 0;
    a2 = OS_MESG_NOBLOCK;
    lib_osSendMesg();
}
