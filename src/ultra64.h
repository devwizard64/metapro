#ifndef _ULTRA64_H_
#define _ULTRA64_H_

#define OS_PRIORITY_IDLE        0

#define OS_EVENT_SW1            0x00
#define OS_EVENT_SW2            0x01
#define OS_EVENT_CART           0x02
#define OS_EVENT_COUNTER        0x03
#define OS_EVENT_SP             0x04
#define OS_EVENT_SI             0x05
#define OS_EVENT_AI             0x06
#define OS_EVENT_VI             0x07
#define OS_EVENT_PI             0x08
#define OS_EVENT_DP             0x09
#define OS_EVENT_CPU_BREAK      0x0A
#define OS_EVENT_SP_BREAK       0x0B
#define OS_EVENT_FAULT          0x0C
#define OS_EVENT_THREADSTATUS   0x0D
#define OS_EVENT_PRENMI         0x0E
#define OS_NUM_EVENTS           0x0F
#define OS_MESG_NOBLOCK         0x00
#define OS_MESG_BLOCK           0x01
#define MQ_RECV                 (mq+0x00)
#define MQ_SEND                 (mq+0x04)
#define MQ_COUNT                (mq+0x08)
#define MQ_INDEX                (mq+0x0C)
#define MQ_LEN                  (mq+0x10)
#define MQ_MSG                  (mq+0x14)

#define CONT_TYPE_NORMAL        0x0005

#define OS_TV_PAL               0
#define OS_TV_NTSC              1
#define OS_TV_MPAL              2

#define M_GFXTASK               1
#define M_AUDTASK               2

#endif /* _ULTRA64_H_ */
