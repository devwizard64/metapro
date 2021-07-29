void lib_osRecvMesg(void)
{
    PTR mq = a0.i[IX];
    PTR thread;
    s32 count;
    s32 index;
    s32 len;
    PTR msg;
    if (__read_s32(MQ_COUNT) == 0)
    {
        if (a2.i[IX] == OS_MESG_NOBLOCK)
        {
            v0.ll = (s32)-1;
            return;
        }
        __write_u32(MQ_RECV, lib_thread->addr);
        lib_thread->ready = false;
        thread_yield(THREAD_YIELD_QUEUE);
    }
    thread = __read_s32(MQ_SEND);
    count  = __read_s32(MQ_COUNT);
    index  = __read_s32(MQ_INDEX);
    len    = __read_s32(MQ_LEN);
    msg    = __read_s32(MQ_MSG);
    __write_u32(MQ_SEND, NULLPTR);
    if (a1.i[IX] != NULLPTR)
    {
        __write_u32(a1.i[IX], __read_s32(msg + 4*index));
    }
    __write_u32(MQ_INDEX, (index+1) % len);
    __write_u32(MQ_COUNT, count-1);
    if (thread != NULLPTR)
    {
        thread_find(thread)->ready = true;
        if (lib_thread != NULL)
        {
            thread_yield(THREAD_YIELD_QUEUE);
        }
    }
    v0.ll = (s32)0;
}
