#define TH_STACK_SIZE   0x10000

static THREAD *os_thread_list  = NULL;
static THREAD *os_thread_queue = NULL;
THREAD *os_thread = NULL;

#if 0
static void th_print(void)
{
    THREAD *queue = os_thread_queue;
    while (queue != NULL)
    {
        pdebug(
            "entry=%08" FMT_X "  pri=%3d  r=%d  id=%d\n",
            queue->entry,
            queue->pri,
            queue->ready,
            queue->id
        );
        queue = queue->qnext;
    }
}
#endif

static void th_llink(THREAD *th)
{
    THREAD **list = &os_thread_list;
    THREAD  *prev = NULL;
    while (*list != NULL)
    {
        if (*list == th) return;
        prev = *list;
        list = &prev->lnext;
    }
    *list = th;
    th->lprev = prev;
    th->lnext = NULL;
}

static void th_qlink(THREAD *th)
{
    THREAD **queue = &os_thread_queue;
    THREAD  *prev  = NULL;
    while (*queue != NULL)
    {
        if (*queue == th) return;
        prev = *queue;
        queue = &prev->qnext;
    }
    *queue = th;
    th->qprev = prev;
    th->qnext = NULL;
    th->qlink = TRUE;
}

static void th_lunlink(THREAD *th)
{
    if (th->lprev != NULL) th->lprev->lnext = th->lnext;
    else                   os_thread_list   = th->lnext;
    if (th->lnext != NULL) th->lnext->lprev = th->lprev;
}

static void th_qunlink(THREAD *th)
{
    if (th->qlink)
    {
        th->qlink = FALSE;
        if (th->qprev != NULL) th->qprev->qnext = th->qnext;
        else                   os_thread_queue  = th->qnext;
        if (th->qnext != NULL) th->qnext->qprev = th->qprev;
    }
}

THREAD *th_find(PTR addr)
{
    THREAD *th;
    if (addr == NULLPTR) return os_thread;
    for (th = os_thread_list; th != NULL && th->addr != addr; th = th->lnext);
    return th;
}

void th_create(PTR addr, s32 id, PTR entry, s32 arg, s32 s, u32 pri)
{
    THREAD *th = th_find(addr);
    if (th == NULL) th_llink(th = malloc(sizeof(THREAD)));
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
    if (th != NULL)
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
    if (os_thread == NULL || setjmp(os_thread->jmp) == 0) longjmp(sys_jmp, arg);
}
