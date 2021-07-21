void lib_osSendMesg(void)
{
    PTR mq = a0.i[IX];
    PTR thread;
    s32 count;
    s32 index;
    s32 len;
    PTR msg;
    if (__read_s32(MQ_COUNT) >= __read_s32(MQ_LEN))
    {
        if (a2.i[IX] == OS_MESG_NOBLOCK)
        {
            v0.ll = (s32)-1;
            return;
        }
        __write_u32(MQ_SEND, lib_thread->addr);
        lib_thread->ready = false;
        thread_yield(THREAD_YIELD_QUEUE);
    }
    thread = __read_u32(MQ_RECV);
    count  = __read_s32(MQ_COUNT);
    index  = __read_s32(MQ_INDEX);
    len    = __read_s32(MQ_LEN);
    msg    = __read_u32(MQ_MSG);
    __write_u32(MQ_RECV, 0);
    __write_u32(msg + 4*((index+count) % len), a1.i[IX]);
    __write_u32(MQ_COUNT, count+1);
    if (thread != 0)
    {
        thread_find(thread)->ready = true;
        if (lib_thread != NULL)
        {
            thread_yield(THREAD_YIELD_QUEUE);
        }
    }
    v0.ll = (s32)0;
}
