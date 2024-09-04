static TIMER *os_timer_list = NULL;

#if 0
static void tm_print(void)
{
	TIMER *list;
	for (list = os_timer_list; list; list = list->next) pdebug(
		"addr=%08" FMT_X "  c=%u  i=%u  event=%08" FMT_X ",%" FMT_d "\n",
		list->addr,
		(uint)(list->countdown/46875),
		(uint)(list->interval /46875),
		__ptr(list->event.mq), list->event.msg
	);
}
#endif

static void tm_link(TIMER *tm)
{
	TIMER **list;
	TIMER *prev = NULL;
	for (list = &os_timer_list; *list; list = &prev->next)
	{
		if (*list == tm) return;
		prev = *list;
	}
	*list = tm;
	tm->prev = prev;
	tm->next = NULL;
}

static void tm_unlink(TIMER *tm)
{
	if (tm->prev)   tm->prev->next = tm->next;
	else            os_timer_list  = tm->next;
	if (tm->next)   tm->next->prev = tm->prev;
}

TIMER *tm_find(PTR addr)
{
	TIMER *tm;
	for (tm = os_timer_list; tm && tm->addr != addr; tm = tm->next);
	return tm;
}

void tm_create(PTR addr, u64 countdown, u64 interval, PTR mq, PTR msg)
{
	TIMER *tm;
	if (!(tm = tm_find(addr))) tm_link(tm = malloc(sizeof(TIMER)));
	tm->addr      = addr;
	tm->countdown = countdown > 0 ? countdown : interval;
	tm->interval  = interval;
	tm->event.mq  = mq;
	tm->event.msg = msg;
}

void tm_destroy(TIMER *tm)
{
	if (tm)
	{
		tm_unlink(tm);
		free(tm);
	}
}

static void tm_update(void)
{
	TIMER *tm;
	TIMER *next;
	for (tm = os_timer_list; tm; tm = next)
	{
		next = tm->next;
		if (tm->countdown >= 781250)
		{
			tm->countdown -= 781250;
		}
		else
		{
			os_event(&tm->event);
			if (tm->interval > 0)   tm->countdown += tm->interval;
			else                    tm_destroy(tm);
		}
	}
}
