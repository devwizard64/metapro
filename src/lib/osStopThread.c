void lib_osStopThread(void)
{
    thread_stop(thread_find(a0.i[IX]));
}
