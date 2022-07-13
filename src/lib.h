#ifndef __LIB_H__
#define __LIB_H__

#include "types.h"
#include "app.h"
#include "sys.h"
#include "cpu.h"

#include "ultra64.h"

#ifndef __ASSEMBLER__

#define LIB_SE(f) static inline void lib_##f(void) {edebug(#f "\n");}

#ifndef __CartRomHandle
#define __CartRomHandle         0
#endif
#ifndef __DriveRomHandle
#define __DriveRomHandle        0
#endif

/* UNSME0 */
#define lib_osSetTime()
#define lib_osMapTLB()
#define lib_osUnmapTLBAll()
#define lib_osCreateMesgQueue() {mesg_init(cpu_ptr(a0), a1, a2);}
#define lib_osSetEventMesg()        \
{                                   \
    __osEventStateTab[a0].mq  = a1; \
    __osEventStateTab[a0].msg = a2; \
}
#define lib_osViSetEvent()                      \
{                                               \
    __osEventStateTab[OS_EVENT_VI].mq  = a0;    \
    __osEventStateTab[OS_EVENT_VI].msg = a1;    \
}
#define lib_osCreateThread() \
    {thread_init(a0, a1, a2, a3, *cpu_s32(sp+0x10), *cpu_s32(sp+0x14));}
#define lib_osRecvMesg() {v0 = mesg_recv(cpu_ptr(a0), a1, a2);}
#define lib_osSpTaskLoad()
extern void lib_osSpTaskStartGo(void);
#define lib_osSpTaskYield()
#define lib_osSendMesg() {v0 = mesg_send(cpu_ptr(a0), a1, a2);}
#define lib_osSpTaskYielded() {v0 = 0;}
#define lib_osStartThread() {thread_start(thread_find(a0));}
#define lib_osWritebackDCacheAll()
#define lib_osCreateViManager()
#define lib_osViSetMode()
#define lib_osViBlack()
#define lib_osViSetSpecialFeatures()
#define lib_osCreatePiManager()
#define lib_osSetThreadPri() \
    {thread_find(a0)->pri = a1; thread_yield(THREAD_YIELD_QUEUE);}
extern void lib_osInitialize(void);
#define lib_osViSwapBuffer() {video_buf = a0;}
#define lib_osContStartReadData() \
    {v0 = mesg_send(cpu_ptr(a0), 0, OS_MESG_NOBLOCK);}
extern void lib_osContGetReadData(void);
extern void lib_osContInit(void);
#define lib_osEepromProbe() {v0 = EEPROM;}
#define lib___ull_rem()                 \
{                                       \
    u64 _a = (u64)a0 << 32 | (u32)a1;   \
    u64 _b = (u64)a2 << 32 | (u32)a3;   \
    u64 _x = _a % _b;                   \
    v0 = _x >> 32;                      \
    v1 = _x >>  0;                      \
}
#define lib___ull_div()                 \
{                                       \
    u64 _a = (u64)a0 << 32 | (u32)a1;   \
    u64 _b = (u64)a2 << 32 | (u32)a3;   \
    u64 _x = _a / _b;                   \
    v0 = _x >> 32;                      \
    v1 = _x >>  0;                      \
}
#define lib___ll_lshift()               \
{                                       \
    s64 _a = (s64)a0 << 32 | (u32)a1;   \
    s64 _b = (s64)a2 << 32 | (u32)a3;   \
    s64 _x = _a << _b;                  \
    v0 = _x >> 32;                      \
    v1 = _x >>  0;                      \
}
#define lib___ll_div()                  \
{                                       \
    s64 _a = (s64)a0 << 32 | (u32)a1;   \
    s64 _b = (s64)a2 << 32 | (u32)a3;   \
    s64 _x = _a / _b;                   \
    v0 = _x >> 32;                      \
    v1 = _x >>  0;                      \
}
#define lib___ll_mul()                  \
{                                       \
    s64 _a = (s64)a0 << 32 | (u32)a1;   \
    s64 _b = (s64)a2 << 32 | (u32)a3;   \
    s64 _x = _a * _b;                   \
    v0 = _x >> 32;                      \
    v1 = _x >>  0;                      \
}
#define lib_osInvalDCache()
extern void lib_osPiStartDma(void);
#define lib_osInvalICache()
#define lib_osEepromLongRead() \
    {eeprom_read(cpu_ptr(a2), a1, a3); v0 = 0;}
#define lib_osEepromLongWrite() \
    {eeprom_write(a1, cpu_ptr(a2), a3); v0 = 0;}
extern void lib_guOrtho(void);
extern void lib_guPerspective(void);
#define lib_osGetTime() {v0 = 0; v1 = 0;}
LIB_SE(__d_to_ull)
LIB_SE(__ull_to_d)
#define lib_osAiSetFrequency() {v0 = AUDIO_FREQ;}
#define lib_osWritebackDCache()
#define lib_osAiGetLength() {v0 = audio_size();}
#define lib_osAiSetNextBuffer() {audio_update(cpu_ptr(a0), a1); v0 = 0;}
#define lib_osVirtualToPhysical() {v0 = __tlb(a0);}

/* UNSMC3 */
#define lib_UNSMC3_80304B38()
#define lib_osGetCount() {v0 = 0;}
LIB_SE(__osGetCurrFaultedThread)
#define lib___osDisableInt() {v0 = 0;}
#define lib___osRestoreInt() {}
extern void lib_osEPiStartDma(void);
#define lib_osCartRomInit() {v0 = __CartRomHandle;}
#define lib_UNSMC3_80308D10() {v0 = 5;}
#define lib_UNSMC3_80308D18() {v0 = 11;}

/* UNKTE0 */
#define lib_osPfsIsPlug() {*cpu_u8(a1) = 0x00; v0 = 0;}
LIB_SE(osPfsInitPak)
LIB_SE(osPfsNumFiles)
LIB_SE(osPfsFileState)
LIB_SE(osPfsFreeBlocks)
LIB_SE(osSyncPrintf)
LIB_SE(osPfsFindFile)
LIB_SE(osPfsDeleteFile)
LIB_SE(osPfsReadWriteFile)
LIB_SE(osPfsAllocateFile)

/* UCZLJ0 */
#define lib_osDriveRomInit() {v0 = __DriveRomHandle;}
LIB_SE(__osPiGetAccess)
LIB_SE(__osPiRelAccess)
#define lib_osDestroyThread() {thread_destroy(thread_find(a0));}
#define lib_osViGetCurrentFramebuffer() {v0 = video_buf;}
#define lib_osGetThreadId() {v0 = thread_find(a0)->id;}
#define lib_osSetIntMask() {v0 = 0;}
#define lib_osGetMemSize() {v0 = MIN(0x800000, CPU_DRAM_SIZE);}
#define lib_osEPiReadIo() {dma(cpu_ptr(a2), a1, 4); v0 = 0;}
#define lib_osSetTimer()                                    \
{                                                           \
    timer_init(                                             \
        a0, (u64)a2 << 32 | (u32)a3,                        \
        (u64)*cpu_u32(sp+0x10) << 32 | *cpu_u32(sp+0x14),   \
        *cpu_u32(sp+0x18), *cpu_u32(sp+0x1C)                \
    );                                                      \
}

#define lib___osMotorAccess() {v0 = 0;}
#define lib_osMotorInit() {v0 = 0;}
#define lib_osStopTimer() {timer_destroy(timer_find(a0));}
#define lib_osAfterPreNMI() {v0 = 0;}
#define lib_osContStartQuery() \
    {v0 = mesg_send(cpu_ptr(a0), 0, OS_MESG_NOBLOCK);}
extern void lib_osContGetQuery(void);
LIB_SE(__osGetActiveQueue)
LIB_SE(osDpGetStatus)
LIB_SE(osDpSetStatus)
#define lib_osContSetCh() {v0 = 0;}
LIB_SE(__osSpGetStatus)
LIB_SE(__osSpSetStatus)
LIB_SE(__ull_to_f)
#define lib_osViSetYScale()

LIB_SE(UCZLJ0_801C9E28)
LIB_SE(UCZLJ0_801C9EC0)
LIB_SE(UCZLJ0_801C9F90)
LIB_SE(UCZLJ0_801C9FFC)
LIB_SE(UCZLJ0_801CA030)
LIB_SE(UCZLJ0_801CA070)
LIB_SE(UCZLJ0_801CA1F0)
LIB_SE(UCZLJ0_801CA740)
LIB_SE(LeoReset)
LIB_SE(LeoResetClear)
LIB_SE(LeoLBAToByte)
LIB_SE(UCZLJ0_801CC190)
LIB_SE(LeoSpdlMotor)
#define lib_LeoDriveExist() {v0 = 0;}
LIB_SE(UCZLJ0_801CE630)
LIB_SE(LeoByteToLBA)
LIB_SE(UCZLJ0_801CF0B0)
LIB_SE(UCZLJ0_801CFBB0)
LIB_SE(LeoCJCreateLeoManager)
LIB_SE(LeoCACreateLeoManager)
LIB_SE(UCZLJ0_801D2CB0)

/* UNK4E0 */
#define lib_osStopThread() {thread_stop(thread_find(a0));}
#define lib_osEepromWrite() \
    {eeprom_write(a1, a2, 8); v0 = 0;}
#define lib_osViGetNextFramebuffer() {v0 = video_buf;}
#define lib_osEPiLinkHandle() {v0 = 0;}
#define lib_osGetThreadPri() {v0 = thread_find(a0)->pri;}
LIB_SE(osDpSetNextBuffer)
LIB_SE(osEepromRead)

#endif /* __ASSEMBLER__ */

#endif /* __LIB_H__ */
