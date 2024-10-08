__OSEventState __osEventStateTab[OS_NUM_EVENTS] = {0};

#if 0
static void mesg_print(OSMesgQueue *mq)
{
	int i;
	int n;
	pdebug("recv:%08X  send:%08X  msg:", mq->recv, mq->send);
	for (i = mq->index, n = mq->count; n != 0; i++, n--)
	{
		pdebug(" %d", *cpu_u32(mq->msg + 4*(i % mq->len)));
	}
	pdebug("\n");
}
#endif

void mesg_create(OSMesgQueue *mq, PTR msg, s32 count)
{
	mq->recv  = NULLPTR;
	mq->send  = NULLPTR;
	mq->count = 0;
	mq->index = 0;
	mq->len   = count;
	mq->msg   = msg;
}

int mesg_send(OSMesgQueue *mq, PTR msg, int flag)
{
	while (mq->count >= mq->len)
	{
		if (flag == OS_MESG_NOBLOCK) return -1;
		if (mq->send) edebug("illegal SendMesgBLOCK");
		mq->send = os_thread->addr;
		os_thread->ready = FALSE;
		th_yield(TH_QUEUE);
	}
	*cpu_s32(mq->msg + 4*((mq->index+mq->count) % mq->len)) = msg;
	mq->count++;
	if (mq->recv)
	{
		th_find(mq->recv)->ready = TRUE;
		mq->recv = NULLPTR;
		if (os_thread) th_yield(TH_QUEUE);
	}
	return 0;
}

int mesg_recv(OSMesgQueue *mq, PTR msg, int flag)
{
	while (mq->count == 0)
	{
		if (flag == OS_MESG_NOBLOCK) return -1;
		if (mq->recv) edebug("illegal RecvMesgBLOCK");
		mq->recv = os_thread->addr;
		os_thread->ready = FALSE;
		th_yield(TH_QUEUE);
	}
	if (msg) *cpu_s32(msg) = *cpu_s32(mq->msg + 4*mq->index);
	mq->index = (mq->index+1) % mq->len;
	mq->count--;
	if (mq->send)
	{
		th_find(mq->send)->ready = TRUE;
		mq->send = NULLPTR;
		if (os_thread) th_yield(TH_QUEUE);
	}
	return 0;
}

void os_event(__OSEventState *es)
{
	if (es->mq) mesg_send(cpu_ptr(es->mq), es->msg, OS_MESG_NOBLOCK);
}
