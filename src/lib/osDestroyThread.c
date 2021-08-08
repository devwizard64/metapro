void lib_osDestroyThread(void)
{
    thread_destroy(thread_find(a0));
}
