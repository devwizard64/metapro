#include "types.h"
#include "app.h"
#include "cpu.h"
#include "sys.h"
#include "gsp.h"
#include "asp.h"
#include "lib.h"

#include "ultra64.h"

static bool sys_reset = false;
static bool sys_fast  = false;
static bool sys_save  = false;
static bool sys_load  = false;
static u8 sys_prenmi = 0;

static jmp_buf sys_jmp;
static jmp_buf sys_nmi;

PTR video_buf = NULLPTR;
u16 video_w = 320;
u16 video_h = 240;
f32 video_l =   0;
f32 video_r = 320;
f32 video_aspect = (float)320/240;

#define video_update_wh(w, h)           \
{                                       \
    video_w = (w);                      \
    video_h = (h);                      \
}

#define video_update_lr()               \
{                                       \
    video_l = 160 - 120*video_aspect;   \
    video_r = 160 + 120*video_aspect;   \
}

#define video_update_size(w, h)         \
{                                       \
    video_update_wh(w, h);              \
    video_aspect = (float)(w)/(h);      \
    video_update_lr();                  \
}

#ifdef __NATIVE__
#include "sys/app.native.c"
#include "sys/rsp.c"
#endif
#ifdef GEKKO
#include "sys/app.gekko.c"
#include "sys/rsp.c"
#endif
#ifdef __NDS__
#include "sys/app.nds.c"
#include "sys/rsp.c"
#endif
#ifdef __3DS__
#include "sys/app.3ds.c"
#include "sys/rsp.3ds.c"
#endif

#include "sys/contdemo.c"
#include "sys/cont.c"
#include "sys/mesg.c"
#include "sys/thread.c"
#include "sys/timer.c"

static void sys_exit(void)
{
    rsp_exit();
    cpu_exit();
    contdemo_exit();
    app_exit();
}

void sys_init(void)
{
    atexit(sys_exit);
    app_init();
    contdemo_init();
    cpu_init();
    rsp_init();
}

static void sys_update(void)
{
    if (sys_save)
    {
        sys_save = false;
        cpu_save();
        contdemo_save();
    }
    if (sys_load)
    {
        sys_load = false;
        cpu_load();
        contdemo_load();
    }
    /* if (video_buf != NULLPTR) gsp_image(__dram(video_buf)); */
    if (!sys_fast) sleep_frame();
    app_update();
    if (sys_prenmi > 0)
    {
        if (--sys_prenmi == 0)
        {
            longjmp(sys_nmi, 1);
        }
    }
    else if (sys_reset)
    {
        sys_reset  = false;
        sys_prenmi = 30;
        os_event(&os_event_table[OS_EVENT_PRENMI]);
    }
    timer_update();
    os_event(&os_event_vi);
    longjmp(sys_jmp, THREAD_YIELD_QUEUE);
}

void sys_main(void (*start)(void))
{
    int arg = setjmp(sys_jmp);
    if (arg != THREAD_YIELD_NULL)
    {
        struct os_thread *queue;
        struct os_thread *thread;
        s32 pri;
        thread = os_thread;
        os_thread = NULL;
        if (arg == THREAD_YIELD_DESTROY)
        {
            thread_destroy(thread);
            thread = NULL;
        }
        if (thread != NULL) memcpy(&thread->cpu, &cpu, sizeof(cpu));
        if (arg == THREAD_YIELD_BREAK) sys_update();
        queue = os_thread_queue;
        thread = NULL;
        pri = OS_PRIORITY_IDLE;
        while (queue != NULL)
        {
            if (queue->ready && pri < queue->pri)
            {
                thread = queue;
                pri = queue->pri;
            }
            queue = queue->qnext;
        }
        if (pri == OS_PRIORITY_IDLE) sys_update();
        os_thread = thread;
        memcpy(&cpu, &thread->cpu, sizeof(cpu));
        if (thread->init)
        {
            register void *stack;
            thread->init = false;
            stack = thread->stack + THREAD_STACK_SIZE-THREAD_STACK_END;
        #ifdef __GNUC__
            asm volatile(
            #ifdef __i386__
                "mov %[stack], %%esp"
            #endif
            #ifdef __x86_64__
                "mov %[stack], %%rsp"
            #endif
            #ifdef __PPC__
                "mr 1, %[stack]"
            #endif
            #ifdef __arm__
                "mov sp, %[stack]"
            #endif
                : [stack] "+r" (stack) ::
            );
        #else
        #error add asm here
        #endif
            __call(os_thread->entry);
            thread_destroy(os_thread);
        }
        else
        {
            longjmp(thread->jmp, 1);
        }
    }
    if (setjmp(sys_nmi) != 0)
    {
        os_thread = NULL;
        while (os_thread_list != NULL) thread_destroy(os_thread_list);
        while (os_timer_list  != NULL) timer_destroy(os_timer_list);
    }
    start();
}

void eexit(void)
{
#ifndef __NATIVE__
    uint i;
    for (i = 0; i < 180; i++) sleep_frame();
#endif
    exit(EXIT_FAILURE);
}
