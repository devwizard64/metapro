#include "types.h"
#include "tm.h"

#ifdef DEBUG

#ifdef __NATIVE__
#define tm_time()       clock()
#define TM_CLOCK        CLOCKS_PER_SEC
typedef clock_t TM_TIME;
#endif
#if defined(GEKKO) || defined(__NDS__)
#define tm_time()       0
#define TM_CLOCK        0
typedef u8 TM_TIME;
#endif
#ifdef __3DS__
#define tm_time()       svcGetSystemTick()
#define TM_CLOCK        SYSCLOCK_ARM11
typedef u64 TM_TIME;
#endif

static const char str_tm_time[TM_LEN][12] =
{
    "CPU THREAD4",
    "CPU THREAD5",
    "RSP AUDTASK",
    "RSP GFXTASK",
};

static TM_TIME tm_table[TM_LEN] = {0};
static TM_TIME tm_cpu_stack[8]  = {0};
static TM_TIME tm_gsp_stack[8]  = {0};
static TM_TIME tm_asp_stack[8]  = {0};
static u8 tm_cpu_index = 0;
static u8 tm_gsp_index = 0;
static u8 tm_asp_index = 0;

void tm_cpu_start(void)
{
    tm_cpu_stack[tm_cpu_index++] = tm_time();
}

void tm_gsp_start(void)
{
    tm_gsp_stack[tm_gsp_index++] = tm_time();
}

void tm_asp_start(void)
{
    tm_asp_stack[tm_asp_index++] = tm_time();
}

void tm_cpu_end(TM_INDEX index)
{
    tm_table[index] += tm_time() - tm_cpu_stack[--tm_cpu_index];
}

void tm_gsp_end(TM_INDEX index)
{
    tm_table[index] += tm_time() - tm_gsp_stack[--tm_gsp_index];
}

void tm_asp_end(TM_INDEX index)
{
    tm_table[index] += tm_time() - tm_asp_stack[--tm_asp_index];
}

void tm_update(void)
{
    memset(tm_table, 0, sizeof(tm_table));
}

void tm_draw(void)
{
    int i;
    pdebug("********************************\n");
    for (i = 0; i < TM_LEN; i++)
    {
        u32 usec = 1000000 * tm_table[i]/TM_CLOCK;
        pdebug("%s:%8" FMT_u "u\n", str_tm_time[i], usec);
    }
}

#endif
