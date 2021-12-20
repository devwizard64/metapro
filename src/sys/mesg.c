struct os_event os_event_table[OS_NUM_EVENTS] = {0};
struct os_event os_event_vi = {0};

#if 0
static void mesg_print(struct os_mesg_queue *mq)
{
    int i;
    int n;
    pdebug("recv:%08X  send:%08X  msg:", mq->recv, mq->send);
    for (i = mq->index, n = mq->count; n != 0; i++, n--)
    {
        pdebug(" %d", *__u32(mq->msg + 4*(i % mq->len)));
    }
    pdebug("\n");
}
#endif

void mesg_init(struct os_mesg_queue *mq, PTR msg, s32 count)
{
    mq->recv  = NULLPTR;
    mq->send  = NULLPTR;
    mq->count = 0;
    mq->index = 0;
    mq->len   = count;
    mq->msg   = msg;
}

int mesg_send(struct os_mesg_queue *mq, PTR msg, int flag)
{
    while (mq->count >= mq->len)
    {
        if (flag == OS_MESG_NOBLOCK) return -1;
        if (mq->send != NULLPTR) edebug("illegal SendMesgBLOCK");
        mq->send = os_thread->addr;
        os_thread->ready = false;
        thread_yield(THREAD_YIELD_QUEUE);
    }
    *__s32(mq->msg + 4*((mq->index+mq->count) % mq->len)) = msg;
    mq->count++;
    if (mq->recv != NULLPTR)
    {
        thread_find(mq->recv)->ready = true;
        mq->recv = NULLPTR;
        if (os_thread != NULL) thread_yield(THREAD_YIELD_QUEUE);
    }
    return 0;
}

int mesg_recv(struct os_mesg_queue *mq, PTR msg, int flag)
{
    while (mq->count == 0)
    {
        if (flag == OS_MESG_NOBLOCK) return -1;
        if (mq->recv != NULLPTR) edebug("illegal RecvMesgBLOCK");
        mq->recv = os_thread->addr;
        os_thread->ready = false;
        thread_yield(THREAD_YIELD_QUEUE);
    }
    if (msg != NULLPTR) *__s32(msg) = *__s32(mq->msg + 4*mq->index);
    mq->index = (mq->index+1) % mq->len;
    mq->count--;
    if (mq->send != NULLPTR)
    {
        thread_find(mq->send)->ready = true;
        mq->send = NULLPTR;
        if (os_thread != NULL) thread_yield(THREAD_YIELD_QUEUE);
    }
    return 0;
}

void os_event(struct os_event *event)
{
    if (event->mq != NULL) mesg_send(event->mq, event->msg, OS_MESG_NOBLOCK);
}
