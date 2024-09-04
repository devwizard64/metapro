#define TH_STACK_SIZE   0x4000

static THREAD *os_thread_list  = NULL;
static THREAD *os_thread_queue = NULL;
THREAD *os_thread = NULL;

#if 0
static void th_print(void)
{
	THREAD *queue;
	for (queue = os_thread_queue; queue; queue = queue->qnext) pdebug(
		"entry=%08" FMT_X "  pri=%3d  r=%d  id=%d\n",
		queue->entry,
		queue->pri,
		queue->ready,
		queue->id
	);
}
#endif

static void th_llink(THREAD *th)
{
	THREAD **list;
	THREAD *prev = NULL;
	for (list = &os_thread_list; *list; list = &prev->lnext)
	{
		if (*list == th) return;
		prev = *list;
	}
	*list = th;
	th->lprev = prev;
	th->lnext = NULL;
}

static void th_qlink(THREAD *th)
{
	THREAD **queue;
	THREAD *prev = NULL;
	for (queue = &os_thread_queue; *queue; queue = &prev->qnext)
	{
		if (*queue == th) return;
		prev = *queue;
	}
	*queue = th;
	th->qprev = prev;
	th->qnext = NULL;
	th->qlink = TRUE;
}

static void th_lunlink(THREAD *th)
{
	if (th->lprev)  th->lprev->lnext = th->lnext;
	else            os_thread_list   = th->lnext;
	if (th->lnext)  th->lnext->lprev = th->lprev;
}

static void th_qunlink(THREAD *th)
{
	if (th->qlink)
	{
		th->qlink = FALSE;
		if (th->qprev)  th->qprev->qnext = th->qnext;
		else            os_thread_queue  = th->qnext;
		if (th->qnext)  th->qnext->qprev = th->qprev;
	}
}

THREAD *th_find(PTR addr)
{
	THREAD *th;
	if (!addr) return os_thread;
	for (th = os_thread_list; th && th->addr != addr; th = th->lnext);
	return th;
}

void th_create(PTR addr, s32 id, PTR entry, s32 arg, s32 s, u32 pri)
{
	THREAD *th;
	if (!(th = th_find(addr))) th_llink(th = malloc(sizeof(THREAD)));
#ifdef __PPC__
	th->stack   = memalign(32, TH_STACK_SIZE);
#else
	th->stack   = malloc(TH_STACK_SIZE);
#endif
	th->a0      = arg;
	th->sp      = s-16;
	th->init    = TRUE;
	th->ready   = TRUE;
	th->qlink   = FALSE;
	th->addr    = addr;
	th->entry   = entry;
	th->id      = id;
	th->pri     = pri;
}

void th_destroy(THREAD *th)
{
	if (th)
	{
		if (th == os_thread)
		{
			longjmp(sys_jmp, TH_DESTROY);
		}
		else
		{
			th_qunlink(th);
			th_lunlink(th);
			free(th->stack);
			free(th);
		}
	}
}

void th_start(THREAD *th)
{
	th_qlink(th);
	th_yield(TH_QUEUE);
}

void th_stop(THREAD *th)
{
	th_qunlink(th);
	th_yield(TH_QUEUE);
}

void th_yield(int arg)
{
	if (!os_thread || !setjmp(os_thread->jmp)) longjmp(sys_jmp, arg);
}
