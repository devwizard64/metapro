void lib_osContStartReadData(void)
{
    a1.i[IX] = 0;
    a2.i[IX] = OS_MESG_NOBLOCK;
    lib_osSendMesg();
}
