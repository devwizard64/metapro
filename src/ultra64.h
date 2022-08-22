#ifndef __ULTRA64_H__
#define __ULTRA64_H__

#define OS_PRIORITY_IDLE        0

#define OS_EVENT_SW1            0
#define OS_EVENT_SW2            1
#define OS_EVENT_CART           2
#define OS_EVENT_COUNTER        3
#define OS_EVENT_SP             4
#define OS_EVENT_SI             5
#define OS_EVENT_AI             6
#define OS_EVENT_VI             7
#define OS_EVENT_PI             8
#define OS_EVENT_DP             9
#define OS_EVENT_CPU_BREAK      10
#define OS_EVENT_SP_BREAK       11
#define OS_EVENT_FAULT          12
#define OS_EVENT_THREADSTATUS   13
#define OS_EVENT_PRENMI         14
#define OS_NUM_EVENTS           15
#define OS_MESG_NOBLOCK         0
#define OS_MESG_BLOCK           1

#define OS_READ                 0
#define OS_WRITE                1

#define MAXCONTROLLERS          4
#define CONT_NO_RESPONSE_ERROR  0x08
#define CONT_TYPE_NORMAL        0x0005

#define OS_TV_PAL               0
#define OS_TV_NTSC              1
#define OS_TV_MPAL              2

#define M_GFXTASK               1
#define M_AUDTASK               2
#define M_NJPEGTASK             4

typedef struct os_mesg_queue
{
    PTR recv;
    PTR send;
    s32 count;
    s32 index;
    s32 len;
    PTR msg;
}
OSMesgQueue;

typedef struct os_cont_status
{
    u16     type;
    u8      status;
    u8      errno_;
}
OSContStatus;

typedef struct os_cont_pad
{
    u16 button;
    s8  stick_x;
    s8  stick_y;
    u8  errno_;
}
OSContPad;

typedef struct os_task
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
}
OSTask;

typedef struct os_event_state
{
    PTR mq;
    PTR msg;
}
__OSEventState;

#endif /* __ULTRA64_H__ */
