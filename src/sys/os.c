#define THREAD_STACK_SIZE       0x4000
#ifdef GEKKO
#define THREAD_STACK_END        0x10
#else
#define THREAD_STACK_END        0
#endif

struct os_thread
{
    struct os_thread *lprev;
    struct os_thread *lnext;
    struct os_thread *qprev;
    struct os_thread *qnext;
    jmp_buf jmp;
    u8     *stack;
    bool    init;
    bool    ready;
    bool    qlink;
    PTR     addr;
    PTR     entry;
    s32     id;
    s32     pri;
    struct cpu cpu;
};

struct os_event
{
    PTR mq;
    PTR msg;
};

struct os_pad
{
    u16 button;
    s8  stick_x;
    s8  stick_y;
    u8  errno_;
};

static struct os_thread *os_thread_list  = NULL;
static struct os_thread *os_thread_queue = NULL;
static struct os_thread *os_thread       = NULL;
static jmp_buf os_jmp;
static jmp_buf os_nmi;
static u8      os_prenmi = 0;

static struct os_event os_event_table[OS_NUM_EVENTS] = {0};
static struct os_event os_event_vi = {0};
static struct os_pad os_pad[4] = {0};

static void thread_llink(struct os_thread *thread)
{
    struct os_thread **list = &os_thread_list;
    struct os_thread  *prev = NULL;
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

static void thread_qlink(struct os_thread *thread)
{
    struct os_thread **queue = &os_thread_queue;
    struct os_thread  *prev  = NULL;
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

static void thread_lunlink(struct os_thread *thread)
{
    if (thread->lprev != NULL)
    {
        thread->lprev->lnext = thread->lnext;
    }
    else
    {
        os_thread_list = thread->lnext;
    }
    if (thread->lnext != NULL)
    {
        thread->lnext->lprev = thread->lprev;
    }
}

static void thread_qunlink(struct os_thread *thread)
{
    if (thread->qlink)
    {
        thread->qlink = false;
        if (thread->qprev != NULL)
        {
            thread->qprev->qnext = thread->qnext;
        }
        else
        {
            os_thread_queue = thread->qnext;
        }
        if (thread->qnext != NULL)
        {
            thread->qnext->qprev = thread->qprev;
        }
    }
}

void thread_yield(int arg)
{
    if (os_thread == NULL || setjmp(os_thread->jmp) == 0)
    {
        longjmp(os_jmp, arg);
    }
}

static struct os_thread *thread_find(PTR addr)
{
    struct os_thread *thread;
    if (addr == NULLPTR)
    {
        thread = os_thread;
    }
    else
    {
        thread = os_thread_list;
        while (thread != NULL && thread->addr != addr) thread = thread->lnext;
    }
    return thread;
}

#if 0
static void thread_print(void)
{
    struct os_thread *queue = os_thread_queue;
    while (queue != NULL)
    {
        printf(
            "entry=%08" FMT_X "  pri=%3d  r=%d  id=%d\n",
            queue->entry, queue->pri, queue->ready, queue->id
        );
        queue = queue->qnext;
    }
}
#endif

s32 thread_id(void)
{
    return os_thread != NULL ? os_thread->id : 0;
}

static void thread_init(PTR addr, s32 id, PTR entry, s32 arg, s32 s, u32 pri)
{
    struct os_thread *thread = malloc(sizeof(*thread));
    thread_llink(thread);
#if defined(__i386__) || defined(__x86_64__)
    thread->stack   = malloc(THREAD_STACK_SIZE);
#endif
#ifdef __PPC__
    thread->stack   = memalign(0x20, THREAD_STACK_SIZE);
#endif
#ifdef __arm__
    thread->stack   = memalign(0x10, THREAD_STACK_SIZE);
#endif
    thread->init    = true;
    thread->ready   = true;
    thread->qlink   = false;
    thread->addr    = addr;
    thread->entry   = entry;
    thread->id      = id;
    thread->pri     = pri;
    thread->a0      = arg;
    thread->sp      = s - 0x10;
}

static void thread_start(struct os_thread *thread)
{
    thread_qlink(thread);
    thread_yield(THREAD_YIELD_QUEUE);
}

#ifdef APP_UNK4
static void thread_stop(struct os_thread *thread)
{
    thread_qunlink(thread);
    thread_yield(THREAD_YIELD_QUEUE);
}
#endif

static void thread_destroy(struct os_thread *thread)
{
    if (thread == os_thread)
    {
        longjmp(os_jmp, THREAD_YIELD_DESTROY);
    }
    else
    {
        thread_qunlink(thread);
        thread_lunlink(thread);
        free(thread->stack);
        free(thread);
    }
}

void thread_fault(void)
{
    if (os_thread != NULL)
    {
        thread_destroy(os_thread);
    }
    else
    {
        exit(EXIT_FAILURE);
    }
}

static void os_event(struct os_event *event)
{
    if (event->mq != NULLPTR)
    {
        a0 = event->mq;
        a1 = event->msg;
        a2 = OS_MESG_NOBLOCK;
        lib_osSendMesg();
    }
}

#define os_cont_status(ptr, i, type, status, errno) \
{                                                   \
    __write_u16((ptr)+4*(i)+0, type);               \
    __write_u8 ((ptr)+4*(i)+2, status);             \
    __write_u8 ((ptr)+4*(i)+3, errno);              \
}
