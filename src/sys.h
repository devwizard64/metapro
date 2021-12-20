#ifndef __SYS_H__
#define __SYS_H__

#include "types.h"
#include "cpu.h"

#define THREAD_YIELD_NULL       0
#define THREAD_YIELD_QUEUE      1
#define THREAD_YIELD_BREAK      2
#define THREAD_YIELD_DESTROY    3

#ifndef __ASSEMBLER__

#define os_cont_status(ptr, i, type, status, errno) \
{                                                   \
    *__u16((ptr)+4*(i)+0) = type;                   \
    *__u8 ((ptr)+4*(i)+2) = status;                 \
    *__u8 ((ptr)+4*(i)+3) = errno;                  \
}

struct os_cont_pad
{
    u16 button;
    s8  stick_x;
    s8  stick_y;
    u8  errno_;
};

struct os_mesg_queue
{
    PTR recv;
    PTR send;
    s32 count;
    s32 index;
    s32 len;
    PTR msg;
};

struct os_event
{
    struct os_mesg_queue *mq;
    PTR msg;
};

struct os_thread
{
    struct os_thread *lprev;
    struct os_thread *lnext;
    struct os_thread *qprev;
    struct os_thread *qnext;
    u8     *stack;
    jmp_buf jmp;
    struct cpu cpu;
    bool    init;
    bool    ready;
    bool    qlink;
    PTR     addr;
    PTR     entry;
    s32     id;
    s32     pri;
};

struct os_timer
{
    struct os_timer *prev;
    struct os_timer *next;
    PTR     addr;
    u64     countdown;
    u64     interval;
    struct os_event event;
};

struct os_task
{
    u32 type;
    u32 flags;
    PTR ucode_boot;
    u32 ucode_boot_size;
    PTR ucode;
    u32 ucode_size;
    PTR ucode_data;
    u32 ucode_data_size;
    PTR dram_stack;
    u32 dram_stack_size;
    PTR output_buff;
    PTR output_buff_size;
    PTR data_ptr;
    u32 data_size;
    PTR yield_data_ptr;
    u32 yield_data_size;
};

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

extern struct os_cont_pad os_cont_pad[];

extern struct os_event os_event_table[];
extern struct os_event os_event_vi;
extern void mesg_init(struct os_mesg_queue *mq, PTR msg, s32 count);
extern int mesg_recv(struct os_mesg_queue *mq, PTR msg, int flag);
extern int mesg_send(struct os_mesg_queue *mq, PTR msg, int flag);
extern void os_event(struct os_event *event);

extern struct os_thread *os_thread;
extern struct os_thread *thread_find(PTR addr);
extern void thread_init(PTR addr, s32 id, PTR entry, s32 arg, s32 s, u32 pri);
extern void thread_destroy(struct os_thread *thread);
extern void thread_start(struct os_thread *thread);
extern void thread_stop(struct os_thread *thread);
extern s32 thread_id(void);
extern void thread_yield(int arg);
extern void thread_fault(void);

extern struct os_timer *timer_find(PTR addr);
extern void timer_init(
    PTR addr, u64 countdown, u64 interval, struct os_mesg_queue *mq, PTR msg
);
extern void timer_destroy(struct os_timer *timer);

extern void sys_init(void);
extern void sys_main(void (*start)(void));

#endif /* __ASSEMBLER__ */

#endif /* __SYS_H__ */
