void lib_osSendMesg(void)
{
    PTR mq = a0;
    PTR thread;
    s32 count;
    s32 index;
    s32 len;
    PTR msg;
    if (__read_s32(MQ_COUNT) >= __read_s32(MQ_LEN))
    {
        if (a2 == OS_MESG_NOBLOCK)
        {
            v0 = -1;
            return;
        }
        __write_u32(MQ_SEND, os_thread->addr);
        os_thread->ready = false;
        thread_yield(THREAD_YIELD_QUEUE);
    }
    thread = __read_s32(MQ_RECV);
    count  = __read_s32(MQ_COUNT);
    index  = __read_s32(MQ_INDEX);
    len    = __read_s32(MQ_LEN);
    msg    = __read_s32(MQ_MSG);
    __write_u32(MQ_RECV, NULLPTR);
    __write_u32(msg + 4*((index+count) % len), a1);
    __write_u32(MQ_COUNT, count+1);
    if (thread != NULLPTR)
    {
        thread_find(thread)->ready = true;
        if (os_thread != NULL) thread_yield(THREAD_YIELD_QUEUE);
    }
    v0 = 0;
}
