#include <setjmp.h>

#include "types.h"
#include "app.h"
#include "cpu.h"
#include "gsp.h"
#include "asp.h"
#include "lib.h"

#include "ultra64.h"

static bool sys_reset = false;
static bool sys_fast  = false;
static bool sys_save  = false;
static bool sys_load  = false;

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

#include "sys/mtx.c"
#include "sys/os.c"
#include "sys/contdemo.c"

#ifdef __NATIVE__
#include "sys/sleep.native.c"
#include "sys/video.native.c"
#include "sys/input.native.c"
#include "sys/audio.native.c"
#include "sys/rsp.c"
#endif
#ifdef GEKKO
#include "sys/sleep.gekko.c"
#include "sys/video.gekko.c"
#include "sys/input.gekko.c"
#include "sys/audio.gekko.c"
#include "sys/rsp.c"
#endif
#ifdef __NDS__
#include "sys/sleep.nds.c"
#include "sys/video.nds.c"
#include "sys/input.nds.c"
#include "sys/audio.nds.c"
#include "sys/rsp.c"
#endif
#ifdef __3DS__
#include "sys/sleep.3ds.c"
#include "sys/video.3ds.c"
#include "sys/input.3ds.c"
#include "sys/audio.3ds.c"
#include "sys/rsp.3ds.c"
#endif

void eexit(void)
{
#ifndef __NATIVE__
    uint i;
    for (i = 0; i < 180; i++) sleep_frame();
#endif
    exit(EXIT_FAILURE);
}

static void lib_update(void)
{
    FILE *f;
#ifdef __NATIVE__
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
            case SDL_QUIT:
                exit(EXIT_SUCCESS);
                break;
            case SDL_WINDOWEVENT:
                switch (event.window.event)
                {
                    case SDL_WINDOWEVENT_RESIZED:
                        SDL_ClearQueuedAudio(audio_device);
                        video_update_size(
                            event.window.data1, event.window.data2
                        );
                        break;
                }
                break;
            case SDL_KEYDOWN:
                switch (event.key.keysym.scancode)
                {
                    case SDL_SCANCODE_F1:   sys_reset = true;       break;
                    case SDL_SCANCODE_F4:   sys_fast ^= false^true; break;
                    case SDL_SCANCODE_F5:   sys_save = true;        break;
                    case SDL_SCANCODE_F7:   sys_load = true;        break;
                    default: break;
                }
                break;
        }
    }
#endif
#ifdef __3DS__
    if (!aptMainLoop()) exit(EXIT_SUCCESS);
#endif
    if (sys_save)
    {
        sys_save = false;
        f = fopen(PATH_DRAM, "wb");
        if (f != NULL)
        {
            fwrite(cpu_dram, 1, sizeof(cpu_dram), f);
            fclose(f);
        }
        else
        {
            wdebug("could not write '" PATH_DRAM "'\n");
        }
    #ifdef INPUT_WRITE
        f = fopen(PATH_INPUT, "wb");
        if (f != NULL)
        {
            fwrite(contdemo_data, 1, contdemo-contdemo_data, f);
            fclose(f);
        }
        else
        {
            wdebug("could not write '" PATH_INPUT "'\n");
        }
    #endif
    }
    if (sys_load)
    {
        sys_load = false;
        f = fopen(PATH_DRAM, "rb");
        if (f != NULL)
        {
            fread(cpu_dram, 1, sizeof(cpu_dram), f);
            fclose(f);
            gsp_cache();
        }
        else
        {
            wdebug("could not read '" PATH_DRAM "'\n");
        }
    #ifdef INPUT_WRITE
        f = fopen(PATH_INPUT, "rb");
        if (f != NULL)
        {
            size_t size;
            fseek(f, 0, SEEK_END);
            size = ftell(f);
            fseek(f, 0, SEEK_SET);
            fread(contdemo_data, 1, size, f);
            fclose(f);
            contdemo = (u8 *)contdemo_data + size;
        }
        else
        {
            wdebug("could not read '" PATH_INPUT "'\n");
        }
    #endif
    }
    if (!sys_fast) sleep_frame();
#ifdef __NATIVE__
    if (SDL_GetQueuedAudioSize(audio_device) > 16384)
    {
        SDL_ClearQueuedAudio(audio_device);
    }
#endif
    if (os_prenmi > 0)
    {
        if (--os_prenmi == 0)
        {
            longjmp(os_nmi, 1);
        }
    }
    else if (sys_reset)
    {
        sys_reset  = false;
        os_prenmi = 30;
        os_event(&os_event_table[OS_EVENT_PRENMI]);
    }
    os_event(&os_event_vi);
    longjmp(os_jmp, THREAD_YIELD_QUEUE);
}

void lib_main(void (*start)(void))
{
    int arg = setjmp(os_jmp);
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
        if (thread != NULL)
        {
            memcpy(&thread->cpu, &cpu, sizeof(cpu));
        }
        if (arg == THREAD_YIELD_BREAK)
        {
            lib_update();
        }
        queue = os_thread_queue;
        thread = NULL;
        pri = OS_PRIORITY_IDLE;
        while (queue != NULL)
        {
            if (queue->ready && queue->pri > pri)
            {
                thread = queue;
                pri = queue->pri;
            }
            queue = queue->qnext;
        }
        if (pri == OS_PRIORITY_IDLE)
        {
            lib_update();
        }
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
    if (setjmp(os_nmi) != 0)
    {
        os_thread = NULL;
        while (os_thread_list != NULL) thread_destroy(os_thread_list);
    }
    start();
}

static void lib_exit(void)
{
    cpu_exit();
    rsp_exit();
    contdemo_exit();
    audio_exit();
    input_exit();
    video_exit();
}

void lib_init(void)
{
    atexit(lib_exit);
    video_init();
    input_init();
    audio_init();
    contdemo_init();
    rsp_init();
    cpu_init();
}

#define LIB_SV(f) void lib_##f(void) {}
#ifdef __DEBUG__
#define LIB_SE(f) void lib_##f(void) {puts(#f); exit(EXIT_FAILURE);}
#else
#define LIB_SE(f) void lib_##f(void) {}
#endif
#define LIB_S0(f) void lib_##f(void) {v0 = 0;}

#ifdef APP_UNSM
#ifdef APP_E0
LIB_SV(osSetTime)
LIB_SV(osMapTLB)
LIB_SV(osUnmapTLBAll)
#include "lib/sprintf.c"
#include "lib/osCreateMesgQueue.c"
#include "lib/osSetEventMesg.c"
#include "lib/osViSetEvent.c"
#include "lib/osCreateThread.c"
#include "lib/osRecvMesg.c"
LIB_SV(osSpTaskLoad)
#include "lib/osSpTaskStartGo.c"
LIB_SV(osSpTaskYield) /* see osSpTaskStartGo */
#include "lib/osSendMesg.c"
LIB_S0(osSpTaskYielded)
#include "lib/osStartThread.c"
LIB_SV(osWritebackDCacheAll)
LIB_SV(osCreateViManager)
LIB_SV(osViSetMode)
LIB_SV(osViBlack)
LIB_SV(osViSetSpecialFeatures)
LIB_SV(osCreatePiManager)
#include "lib/osSetThreadPri.c"
#include "lib/osInitialize.c"
LIB_SV(osViSwapBuffer)
#include "lib/sqrtf.c"
#include "lib/osContStartReadData.c"
#include "lib/osContGetReadData.c"
#include "lib/osContInit.c"
#include "lib/osEepromProbe.c"
LIB_SE(__ull_div)
#include "lib/__ll_lshift.c"
LIB_SE(__ll_mul)
LIB_SV(osInvalDCache)
#include "lib/osPiStartDma.c"
#include "lib/bzero.c"
LIB_SV(osInvalICache)
#include "lib/osEepromLongRead.c"
#include "lib/osEepromLongWrite.c"
#include "lib/bcopy.c"
#include "lib/guOrtho.c"
#include "lib/guPerspective.c"
#include "lib/osGetTime.c"
LIB_SE(__d_to_ull)
LIB_SE(__ull_to_d)
#include "lib/cosf.c"
#include "lib/sinf.c"
#include "lib/guTranslate.c"
#include "lib/guRotate.c"
#include "lib/guScale.c"
#include "lib/osAiSetFrequency.c"
#include "lib/alSeqFileNew.c"
LIB_SV(osWritebackDCache)
#include "lib/osAiGetLength.c"
#include "lib/osAiSetNextBuffer.c"
#include "lib/osVirtualToPhysical.c"
#endif

#ifdef APP_C3
#include "lib/osInitialize.c"
#include "lib/osAiGetLength.c"
#include "lib/osAiSetFrequency.c"
#include "lib/osAiSetNextBuffer.c"
LIB_SV(osInvalDCache)
LIB_SV(osInvalICache)
LIB_SV(osWritebackDCacheAll)
#include "lib/osContStartReadData.c"
#include "lib/osContGetReadData.c"
#include "lib/osContInit.c"
#include "lib/osVirtualToPhysical.c"
#include "lib/sqrtf.c"
#include "lib/sinf.c"
#include "lib/guOrtho.c"
#include "lib/guPerspective.c"
#include "lib/cosf.c"
#include "lib/bcopy.c"
#include "lib/bzero.c"
LIB_SE(__ull_div)
#include "lib/__ll_lshift.c"
LIB_SE(__ll_mul)
LIB_SE(80304094)
LIB_SE(803040C0)
#include "lib/sprintf.c"
/* stub */
LIB_SV(80304B38)
#include "lib/osCreateMesgQueue.c"
#include "lib/osRecvMesg.c"
#include "lib/osSendMesg.c"
#include "lib/osSetEventMesg.c"
/* LIB_SE(80304F80) */
LIB_SV(80304F80)
LIB_SV(osSpTaskLoad)
#include "lib/osSpTaskStartGo.c"
LIB_SV(osSpTaskYield) /* see osSpTaskStartGo */
LIB_S0(osSpTaskYielded)
#include "lib/osCreateThread.c"
#include "lib/osSetThreadPri.c"
#include "lib/osStartThread.c"
LIB_SE(__osGetCurrFaultedThread)
#include "lib/osGetTime.c"
LIB_SV(osSetTime)
LIB_SV(osMapTLB)
LIB_SV(osUnmapTLBAll)
LIB_SV(osCreateViManager)
#include "lib/osViSetEvent.c"
LIB_SV(osViSetMode)
LIB_SV(osViSetSpecialFeatures)
LIB_SV(osViSwapBuffer)
LIB_SV(osViBlack)
#include "lib/guScale.c"
#include "lib/guTranslate.c"
#include "lib/guRotate.c"
#include "lib/osEepromProbe.c"
#include "lib/osEepromLongWrite.c"
#include "lib/osEepromLongRead.c"
LIB_S0(__osDisableInt)
LIB_SV(__osRestoreInt)
LIB_SV(osCreatePiManager)
#include "lib/osEPiStartDma.c"
/* LIB_SE(80308350) */
LIB_SV(80308350)
#include "lib/osPiStartDma.c"

void lib_80308D10(void)
{
    v0 = 5;
}

void lib_80308D18(void)
{
    v0 = 11;
}

/*
LIB_SE(__d_to_ull)
LIB_SE(__ull_to_d)
#include "lib/guTranslate.c"
#include "lib/guRotate.c"
#include "lib/guScale.c"
#include "lib/alSeqFileNew.c"
LIB_SV(osWritebackDCache)
*/
#endif

#ifdef APP_E4
struct meme
{
    u8  type;
    u32 id;
};

#define mq_app_vi   (s32)0x8033B010
#define msg_app_vi  (s32)0x8033B040
#define sc_app      (s32)0x8033B048
void lib_80248AF0(void)
{
    a0 = mq_app_vi;
    a1 = msg_app_vi;
    a2 = 1;
    lib_osCreateMesgQueue();
    a0 = 2;
    a1 = sc_app;
    a2 = mq_app_vi;
    a3 = 1;
    app_80246B14();
    a0 = 2;
    a1 = 0;
    a2 = 0;
    app_803219AC();
    a0 = 0;
    app_80248E08();
    __write_u16(0x8032DDF8, 32);
    if (0 == 0)
    {
        a0 = 0;
        a1 = 0x0021;
        a2 = 0x0000;
        app_803219AC();
    }
    else
    {
        __write_f32(0x80000400, 0);
        __write_f32(0x80000404, 0);
        __write_f32(0x80000408, 0);
        a0 = 0x24228081;
        a1 = 0x80000400;
        app_8031EB00();
    }
    while (true)
    {
        app_802494D8();
        a0 = mq_app_vi;
        a1 = 0;
        a2 = OS_MESG_BLOCK;
        lib_osRecvMesg();
        a0 = mq_app_vi;
        a1 = 0;
        a2 = OS_MESG_BLOCK;
        lib_osRecvMesg();
    }
}

LIB_SV(80278074)
LIB_SV(80278974)
LIB_SV(80278B98)
LIB_SV(8027E490)
LIB_SV(8027E520)
LIB_SV(8027E5CC)
#include "lib/osCreateMesgQueue.c"
#include "lib/osSetEventMesg.c"
#include "lib/osViSetEvent.c"
#include "lib/osCreateThread.c"
#include "lib/osRecvMesg.c"
LIB_SV(osSpTaskLoad)
#include "lib/osSpTaskStartGo.c"
LIB_SV(osSpTaskYield) /* see osSpTaskStartGo */
#include "lib/osSendMesg.c"
LIB_S0(osSpTaskYielded)
#include "lib/osStartThread.c"
LIB_SV(osWritebackDCacheAll)
LIB_SV(osCreateViManager)
LIB_SV(osViSetMode)
LIB_SV(osViBlack)
LIB_SV(osViSetSpecialFeatures)
LIB_SV(osCreatePiManager)
#include "lib/osSetThreadPri.c"
#include "lib/osInitialize.c"
#include "lib/sqrtf.c"
LIB_SV(osInvalDCache)
#include "lib/osPiStartDma.c"
#include "lib/osAiSetFrequency.c"
#include "lib/alSeqFileNew.c"
LIB_SV(osWritebackDCache)
#include "lib/osAiGetLength.c"
#include "lib/osAiSetNextBuffer.c"
#endif
#endif

#ifdef APP_UNKT
#ifdef APP_E0
#include "lib/osCreateThread.c"
#include "lib/osInitialize.c"
#include "lib/osStartThread.c"
LIB_SV(osCreateViManager)
LIB_SV(osViSetMode)
LIB_SV(osViBlack)
LIB_SV(osViSetSpecialFeatures)
LIB_SV(osCreatePiManager)
#include "lib/osSetThreadPri.c"
#include "lib/osCreateMesgQueue.c"
#include "lib/osViSetEvent.c"
#include "lib/osSetEventMesg.c"
LIB_SV(osSpTaskLoad)
#include "lib/osSpTaskStartGo.c"
#include "lib/osContInit.c"
#include "lib/osContStartReadData.c"
#include "lib/osContGetReadData.c"
#include "lib/osRecvMesg.c"
LIB_SV(osWritebackDCacheAll)
#include "lib/osSendMesg.c"
LIB_SV(osViSwapBuffer)
#include "lib/bzero.c"
LIB_SV(osInvalICache)
LIB_SV(osInvalDCache)
#include "lib/osPiStartDma.c"
LIB_SV(osSpTaskYield) /* see osSpTaskStartGo */
LIB_S0(osSpTaskYielded)
#include "lib/osGetTime.c"
LIB_SE(__ull_rem)
LIB_SE(__ull_div)
LIB_SE(__ll_div)
LIB_SE(__ll_mul)
LIB_SE(__osGetCurrFaultedThread)
#include "lib/sqrtf.c"
#include "lib/guOrtho.c"
LIB_SV(osSetTime)
#include "lib/osEepromProbe.c"
#include "lib/osPfsIsPlug.c"
LIB_SE(osPfsInitPak)
LIB_SE(osPfsNumFiles)
LIB_SE(osPfsFileState)
LIB_SE(osPfsFreeBlocks)
#include "lib/guRotate.c"
#include "lib/guScale.c"
#include "lib/guPerspective.c"
#include "lib/guLookAtF.c"
#include "lib/guLookAt.c"
#include "lib/guTranslate.c"
LIB_SE(osSyncPrintf)
#include "lib/guMtxCatL.c"
LIB_SE(osPfsFindFile)
LIB_SE(osPfsDeleteFile)
#include "lib/osEepromLongWrite.c"
#include "lib/osEepromLongRead.c"
LIB_SE(osPfsReadWriteFile)
LIB_SE(osPfsAllocateFile)
#include "lib/osAiSetFrequency.c"
#include "lib/osAiGetLength.c"
#include "lib/osAiSetNextBuffer.c"
LIB_S0(osGetCount)
LIB_SV(osWritebackDCache)
#include "lib/bcopy.c"
#include "lib/osVirtualToPhysical.c"
LIB_SE(osSetTimer)
#include "lib/sinf.c"
#include "lib/cosf.c"
#include "lib/guMtxCatF.c"
#endif
#endif

#ifdef APP_UNK4
#ifdef APP_E0
#include "lib/osSendMesg.c"
#include "lib/osStopThread.c"
#include "lib/osRecvMesg.c"
LIB_S0(osSetIntMask)
#include "lib/sinf.c"
LIB_SV(osSpTaskLoad)
#include "lib/osSpTaskStartGo.c"
#include "lib/osDestroyThread.c"
LIB_SE(__ull_div)
LIB_SE(__ll_mul)
#include "lib/bzero.c"
LIB_S0(__osMotorAccess)
LIB_S0(osMotorInit)
LIB_S0(osContReset)
#include "lib/osEepromWrite.c"
#include "lib/osCreateThread.c"
#include "lib/osContStartReadData.c"
#include "lib/osContGetReadData.c"
#include "lib/osEepromLongRead.c"
#include "lib/osVirtualToPhysical.c"
LIB_SV(osWritebackDCache)
#include "lib/osInitialize.c"
LIB_S0(osViGetNextFramebuffer)
LIB_S0(osEPiLinkHandle)
LIB_SV(osViBlack)
LIB_SE(osSpTaskYield) /* see osSpTaskStartGo */
LIB_SE(80030794)
#include "lib/guMtxIdentF.c"
LIB_SV(osViSetMode)
LIB_SE(osPfsAllocateFile)
LIB_S0(osGetCount)
LIB_SE(osEepromProbe)
LIB_SE(osPfsFindFile)
LIB_SV(osCreatePiManager)
#include "lib/osSetEventMesg.c"
#include "lib/sqrtf.c"
LIB_SE(osAfterPreNMI)
#include "lib/osContStartQuery.c"
#include "lib/osContGetQuery.c"
LIB_SE(_Printf)
#include "lib/osEPiStartDma.c"
#include "lib/memcpy.c"
#include "lib/osCreateMesgQueue.c"
LIB_SV(osInvalICache)
LIB_SV(osInvalDCache)
#include "lib/osEepromLongWrite.c"
#include "lib/osSetThreadPri.c"
LIB_SE(osGetThreadPri)
LIB_SV(osViSwapBuffer)
#include "lib/guMtxXFMF.c"
#include "lib/guMtxCatF.c"
LIB_SE(osSpTaskYielded)
#include "lib/osGetTime.c"
#include "lib/osAiSetFrequency.c"
#include "lib/guNormalize.c"
LIB_SE(__osGetActiveQueue)
#include "lib/alCopy.c"
LIB_SE(osPfsDeleteFile)
#include "lib/cosf.c"
LIB_SE(osSetTime)
#include "lib/osViSetEvent.c"
LIB_S0(osCartRomInit)
LIB_SV(guS2DInitBg)
LIB_SE(80035D30)
#include "lib/alCents2Ratio.c"
LIB_SE(osDpSetNextBuffer)
LIB_SV(osCreateViManager)
LIB_SV(osWritebackDCacheAll)
#include "lib/osStartThread.c"
LIB_SV(osViSetYScale)
#include "lib/osAiSetNextBuffer.c"
LIB_SE(osEepromRead)
LIB_S0(osViGetCurrentFramebuffer)
/* ext */
#include "lib/osAiGetLength.c"
#endif
#endif
