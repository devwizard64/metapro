void lib_osSetThreadPri(void)
{
    thread_find(a0)->pri = a1;
    thread_yield(THREAD_YIELD_QUEUE);
}
