void lib_osSetThreadPri(void)
{
    thread_find(a0.i[IX])->pri = a1.i[IX];
    thread_yield(THREAD_YIELD_QUEUE);
}
