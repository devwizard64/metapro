#define THREAD_STACK_SIZE       0x4000
#ifdef __PPC__
#define THREAD_STACK_END        0x10
#else
#define THREAD_STACK_END        0
#endif

static OSThread *os_thread_list  = NULL;
static OSThread *os_thread_queue = NULL;
OSThread *os_thread = NULL;

#if 0
static void thread_print(void)
{
    OSThread *queue = os_thread_queue;
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

static void thread_llink(OSThread *thread)
{
    OSThread **list = &os_thread_list;
    OSThread  *prev = NULL;
    while (*list != NULL)
    {
        if (*list == thread) return;
        prev = *list;
        list = &prev->lnext;
    }
    *list = thread;
    thread->lprev = prev;
    thread->lnext = NULL;
}

static void thread_qlink(OSThread *thread)
{
    OSThread **queue = &os_thread_queue;
    OSThread  *prev  = NULL;
    while (*queue != NULL)
    {
        if (*queue == thread) return;
        prev = *queue;
        queue = &prev->qnext;
    }
    *queue = thread;
    thread->qprev = prev;
    thread->qnext = NULL;
    thread->qlink = true;
}

static void thread_lunlink(OSThread *thread)
{
    if (thread->lprev != NULL) thread->lprev->lnext = thread->lnext;
    else                       os_thread_list       = thread->lnext;
    if (thread->lnext != NULL) thread->lnext->lprev = thread->lprev;
}

static void thread_qunlink(OSThread *thread)
{
    if (thread->qlink)
    {
        thread->qlink = false;
        if (thread->qprev != NULL) thread->qprev->qnext = thread->qnext;
        else                       os_thread_queue      = thread->qnext;
        if (thread->qnext != NULL) thread->qnext->qprev = thread->qprev;
    }
}

OSThread *thread_find(PTR addr)
{
    OSThread *thread;
    if (addr == NULLPTR) return os_thread;
    thread = os_thread_list;
    while (thread != NULL && thread->addr != addr) thread = thread->lnext;
    return thread;
}

void thread_init(PTR addr, s32 id, PTR entry, s32 arg, s32 s, u32 pri)
{
    OSThread *thread = thread_find(addr);
    if (thread == NULL)
    {
        thread = malloc(sizeof(*thread));
        thread_llink(thread);
    }
#if defined(__i386__) || defined(__x86_64__)
    thread->stack   = malloc(THREAD_STACK_SIZE);
#endif
#ifdef __PPC__
    thread->stack   = memalign(0x20, THREAD_STACK_SIZE);
#endif
#ifdef __arm__
    thread->stack   = memalign(0x10, THREAD_STACK_SIZE);
#endif
    thread->a0      = arg;
    thread->sp      = s - 0x10;
    thread->init    = true;
    thread->ready   = true;
    thread->qlink   = false;
    thread->addr    = addr;
    thread->entry   = entry;
    thread->id      = id;
    thread->pri     = pri;
}

void thread_destroy(OSThread *thread)
{
    if (thread != NULL)
    {
        if (thread == os_thread)
        {
            longjmp(sys_jmp, THREAD_YIELD_DESTROY);
        }
        else
        {
            thread_qunlink(thread);
            thread_lunlink(thread);
            free(thread->stack);
            free(thread);
        }
    }
}

void thread_start(OSThread *thread)
{
    thread_qlink(thread);
    thread_yield(THREAD_YIELD_QUEUE);
}

void thread_stop(OSThread *thread)
{
    thread_qunlink(thread);
    thread_yield(THREAD_YIELD_QUEUE);
}

void thread_yield(int arg)
{
    if (os_thread == NULL || setjmp(os_thread->jmp) == 0) longjmp(sys_jmp, arg);
}
