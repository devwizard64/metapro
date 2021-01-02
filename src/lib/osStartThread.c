void lib_osStartThread(void)
{
    thread_start(thread_find(a0.i[IX]));
}
