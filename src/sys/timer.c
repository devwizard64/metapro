static TIMER *os_timer_list = NULL;

#if 0
static void timer_print(void)
{
    TIMER *list = os_timer_list;
    while (list != NULL)
    {
        pdebug(
            "addr=%08" FMT_X "  c=%d  i=%d  event=%08" FMT_X ",%" FMT_d "\n",
            list->addr,
            (uint)(list->countdown/46875),
            (uint)(list->interval /46875),
            __ptr(list->event.mq), list->event.msg
        );
        list = list->next;
    }
}
#endif

static void timer_link(TIMER *timer)
{
    TIMER **list = &os_timer_list;
    TIMER  *prev = NULL;
    while (*list != NULL)
    {
        if (*list == timer) return;
        prev = *list;
        list = &prev->next;
    }
    *list = timer;
    timer->prev = prev;
    timer->next = NULL;
}

static void timer_unlink(TIMER *timer)
{
    if (timer->prev != NULL) timer->prev->next = timer->next;
    else                     os_timer_list     = timer->next;
    if (timer->next != NULL) timer->next->prev = timer->prev;
}

TIMER *timer_find(PTR addr)
{
    TIMER *timer = os_timer_list;
    while (timer != NULL && timer->addr != addr) timer = timer->next;
    return timer;
}

void timer_init(PTR addr, u64 countdown, u64 interval, OSMesgQueue *mq, PTR msg)
{
    TIMER *timer = timer_find(addr);
    if (timer == NULL)
    {
        timer = malloc(sizeof(*timer));
        timer_link(timer);
    }
    timer->addr      = addr;
    timer->countdown = countdown > 0 ? countdown : interval;
    timer->interval  = interval;
    timer->event.mq  = mq;
    timer->event.msg = msg;
}

void timer_destroy(TIMER *timer)
{
    if (timer != NULL)
    {
        timer_unlink(timer);
        free(timer);
    }
}

static void timer_update(void)
{
    TIMER *timer = os_timer_list;
    while (timer != NULL)
    {
        TIMER *next = timer->next;
        if (timer->countdown >= 781250)
        {
            timer->countdown -= 781250;
        }
        else
        {
            os_event(&timer->event);
            if (timer->interval > 0)
            {
                timer->countdown += timer->interval;
            }
            else
            {
                timer_destroy(timer);
            }
        }
        timer = next;
    }
}
