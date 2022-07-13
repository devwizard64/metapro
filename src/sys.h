#ifndef __SYS_H__
#define __SYS_H__

#include "types.h"
#include "cpu.h"

#include "ultra64.h"

#define THREAD_YIELD_NULL       0
#define THREAD_YIELD_QUEUE      1
#define THREAD_YIELD_BREAK      2
#define THREAD_YIELD_DESTROY    3

#ifndef __ASSEMBLER__

typedef struct thread
{
    struct thread *lprev;
    struct thread *lnext;
    struct thread *qprev;
    struct thread *qnext;
    u8     *stack;
    jmp_buf jmp;
    CPU     cpu;
    bool    init;
    bool    ready;
    bool    qlink;
    PTR     addr;
    PTR     entry;
    s32     id;
    s32     pri;
}
THREAD;

typedef struct timer
{
    struct timer *prev;
    struct timer *next;
    PTR     addr;
    u64     countdown;
    u64     interval;
    __OSEventState event;
}
TIMER;

extern PTR video_buf;
extern u16 video_w;
extern u16 video_h;
extern f32 video_l;
extern f32 video_r;
extern f32 video_aspect;

#ifdef __NATIVE__
extern SDL_Window *window;
#endif
#ifdef GEKKO
extern void *framebuffer;
#endif

extern void input_update(void);

extern void audio_update(void *src, size_t size);
extern s32 audio_size(void);

extern void rsp_gfxtask(PTR ucode, void *data);
extern void rsp_audtask(void *data, u32 size);

extern void contdemo_update(void);

extern OSContStatus os_cont_status[MAXCONTROLLERS];
extern OSContPad os_cont_pad[MAXCONTROLLERS];

extern __OSEventState __osEventStateTab[OS_NUM_EVENTS];
extern __OSEventState __osEventStateVi;
extern void mesg_init(OSMesgQueue *mq, PTR msg, s32 count);
extern int mesg_recv(OSMesgQueue *mq, PTR msg, int flag);
extern int mesg_send(OSMesgQueue *mq, PTR msg, int flag);
extern void os_event(__OSEventState *es);

extern THREAD *os_thread;
extern THREAD *thread_find(PTR addr);
extern void thread_init(PTR addr, s32 id, PTR entry, s32 arg, s32 s, u32 pri);
extern void thread_destroy(THREAD *thread);
extern void thread_start(THREAD *thread);
extern void thread_stop(THREAD *thread);
extern s32 thread_id(void);
extern void thread_yield(int arg);
extern void thread_fault(void);

extern TIMER *timer_find(PTR addr);
extern void timer_init(PTR addr, u64 countdown, u64 interval, PTR mq, PTR msg);
extern void timer_destroy(TIMER *timer);

extern void sys_init(void);
extern void sys_main(void (*start)(void));

#endif /* __ASSEMBLER__ */

#endif /* __SYS_H__ */
