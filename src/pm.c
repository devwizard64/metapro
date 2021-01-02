#include <stdio.h>
#include <string.h>
#ifdef _NATIVE
#include <time.h>
#endif

#include "types.h"
#include "pm.h"

#ifdef _NATIVE
#define pm_time()       clock()
#define PM_USEC         (1000000.0F/CLOCKS_PER_SEC)
typedef clock_t pm_time_t;
#endif
#ifdef _3DS
#define pm_time()       svcGetSystemTick()
#define PM_USEC         (1000000.0F/SYSCLOCK_ARM11)
typedef u64 pm_time_t;
#endif
#ifdef _GCN
#define pm_time()       0
#define PM_USEC         0.0F
typedef u8 pm_time_t;
#endif

static const char *const pm_str[PM_LEN] =
{
    "CPU THREAD4",
    "CPU THREAD5",
    "RSP AUDTASK",
    "RSP GFXTASK",
};

static pm_time_t pm_table[PM_LEN] = {0};
static pm_time_t pm_cpu_stack[8]  = {0};
static pm_time_t pm_rsp_stack[8]  = {0};
static u8 pm_cpu_index = 0;
static u8 pm_rsp_index = 0;

void pm_cpu_start(void)
{
    pm_cpu_stack[pm_cpu_index++] = pm_time();
}

void pm_rsp_start(void)
{
    pm_rsp_stack[pm_rsp_index++] = pm_time();
}

void pm_cpu_end(enum pm_index_t index)
{
    pm_table[index] += pm_time() - pm_cpu_stack[--pm_cpu_index];
}

void pm_rsp_end(enum pm_index_t index)
{
    pm_table[index] += pm_time() - pm_rsp_stack[--pm_rsp_index];
}

void pm_update(void)
{
    memset(pm_table, 0x00, sizeof(pm_table));
}

void pm_draw(void)
{
    uint i;
    puts("********************************");
    for (i = 0; i < PM_LEN; i++)
    {
        u32 usec = PM_USEC*pm_table[i];
        printf("%s:%8" FMT_d "u\n", pm_str[i], usec);
    }
}
