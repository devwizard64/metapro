#ifndef __SYS_H__
#define __SYS_H__

#include "cpu.h"

#include "ultra64.h"

#define TH_RESET        -1
#define TH_QUEUE        1
#define TH_BREAK        2
#define TH_DESTROY      3

typedef struct thread
{
	struct thread *lprev;
	struct thread *lnext;
	struct thread *qprev;
	struct thread *qnext;
	u8 *stack;
	jmp_buf jmp;
	CPU cpu;
	bool init;
	bool ready;
	bool qlink;
	PTR addr;
	PTR entry;
	s32 id;
	s32 pri;
}
THREAD;

typedef struct timer
{
	struct timer *prev;
	struct timer *next;
	PTR addr;
	u64 countdown;
	u64 interval;
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
extern void mesg_create(OSMesgQueue *mq, PTR msg, s32 count);
extern int mesg_recv(OSMesgQueue *mq, PTR msg, int flag);
extern int mesg_send(OSMesgQueue *mq, PTR msg, int flag);
extern void os_event(__OSEventState *es);

extern THREAD *os_thread;
extern THREAD *th_find(PTR addr);
extern void th_create(PTR addr, s32 id, PTR entry, s32 arg, s32 s, u32 pri);
extern void th_destroy(THREAD *th);
extern void th_start(THREAD *th);
extern void th_stop(THREAD *th);
extern s32 th_id(void);
extern void th_yield(int arg);
extern void th_fault(void);

extern TIMER *tm_find(PTR addr);
extern void tm_create(PTR addr, u64 countdown, u64 interval, PTR mq, PTR msg);
extern void tm_destroy(TIMER *tm);

extern void sys_init(void);
extern void sys_main(void (*start)(void));

#endif /* __SYS_H__ */
