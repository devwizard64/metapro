#include "types.h"
#include "app.h"
#include "sys.h"
#include "cpu.h"
#include "rsp.h"
#include "lib.h"

#include "ultra64.h"

static bool sys_reset = FALSE;
static bool sys_fast  = FALSE;
static bool sys_save  = FALSE;
static bool sys_load  = FALSE;
static u8 sys_prenmi = 0;

static jmp_buf sys_jmp;
static jmp_buf sys_nmi;

PTR video_buf = NULLPTR;
u16 video_w = 320;
u16 video_h = 240;
f32 video_l =   0;
f32 video_r = 320;
f32 video_aspect = (float)320/240;

#define video_resize(w, h)              \
{                                       \
    video_w = (w);                      \
    video_h = (h);                      \
    video_l = 160 - 120*video_aspect;   \
    video_r = 160 + 120*video_aspect;   \
}

#define video_resize_1(w, h)            \
{                                       \
    video_aspect = (float)(w)/(h);      \
    video_resize(w, h);                 \
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
        sys_save = FALSE;
        cpu_save();
        contdemo_save();
    }
    if (sys_load)
    {
        sys_load = FALSE;
        cpu_load();
        contdemo_load();
    }
    /* if (video_buf != NULLPTR) gsp_image(cpu_ptr(video_buf)); */
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
        sys_reset  = FALSE;
        sys_prenmi = 30;
        os_event(&__osEventStateTab[OS_EVENT_PRENMI]);
    }
    tm_update();
    os_event(&__osEventStateTab[OS_EVENT_VI]);
    longjmp(sys_jmp, TH_QUEUE);
}

void sys_main(void (*start)(void))
{
    int arg = setjmp(sys_jmp);
    if (arg != 0)
    {
        THREAD *queue;
        THREAD *th;
        s32 pri;
        th = os_thread;
        os_thread = NULL;
        if (arg == TH_DESTROY)
        {
            th_destroy(th);
            th = NULL;
        }
        if (th != NULL) memcpy(&th->cpu, &cpu, sizeof(CPU));
        if (arg == TH_BREAK) sys_update();
        queue = os_thread_queue;
        th = NULL;
        pri = OS_PRIORITY_IDLE;
        while (queue != NULL)
        {
            if (queue->ready && pri < queue->pri)
            {
                th = queue;
                pri = queue->pri;
            }
            queue = queue->qnext;
        }
        if (pri == OS_PRIORITY_IDLE) sys_update();
        os_thread = th;
        memcpy(&cpu, &th->cpu, sizeof(CPU));
        if (th->init)
        {
            void *stack = th->stack + TH_STACK_SIZE - 16;
            th->init = FALSE;
#ifdef __GNUC__
            asm volatile(
#ifdef __i386__
                "mov %[stack], %%esp"
#endif
#ifdef __x86_64__
                "mov %[stack], %%rsp"
#endif
#if defined(__arm__) || defined(__arm64__)
                "mov sp, %[stack]"
#endif
#ifdef __PPC__
                "mr 1, %[stack]"
#endif
                : [stack] "+r" (stack) ::
            );
#else
#error add asm here
#endif
            __call(os_thread->entry);
            th_destroy(os_thread);
        }
        else
        {
            longjmp(th->jmp, 1);
        }
    }
    if (setjmp(sys_nmi) != 0)
    {
        os_thread = NULL;
        while (os_thread_list != NULL) th_destroy(os_thread_list);
        while (os_timer_list  != NULL) tm_destroy(os_timer_list);
    }
    start();
}

void eexit(void)
{
#ifndef __NATIVE__
    int i;
    for (i = 0; i < 180; i++) sleep_frame();
#endif
    exit(1);
}
