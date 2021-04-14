#ifndef _LIB_H_
#define _LIB_H_

#include "types.h"
#include "app.h"

#define THREAD_YIELD_NULL       0
#define THREAD_YIELD_QUEUE      1
#define THREAD_YIELD_BREAK      2
#define THREAD_YIELD_DESTROY    3

#ifndef __ASSEMBLER__

#define MDOT3(m, i) ((m)[0][i]*x + (m)[1][i]*y + (m)[2][i]*z)
#define IDOT3(m, i) ((m)[i][0]*x + (m)[i][1]*y + (m)[i][2]*z)
#define MDOT4(m, i) (MDOT3(m, i) + (m)[3][i])

extern u16 lib_video_w;
extern u16 lib_video_h;
extern f32 lib_viewport_l;
extern f32 lib_viewport_r;

extern void mtx_write(s16 *, const f32 *);
extern void mtxf_mul(f32[4][4], f32[4][4], f32[4][4]);
extern void mtxf_identity(f32[4][4]);
extern void mtxf_ortho(f32[4][4], f32, f32, f32, f32, f32, f32);
extern void thread_yield(int);
extern void thread_fault(void);
extern void video_update(void);
extern void lib_main(void (*)(void));
extern void lib_init(void);

#ifdef APP_UNSM
#ifdef APP_E0
extern void lib_osSetTime(void);
extern void lib_osMapTLB(void);
extern void lib_osUnmapTLBAll(void);
extern void lib_sprintf(void);
extern void lib_osCreateMesgQueue(void);
extern void lib_osSetEventMesg(void);
extern void lib_osViSetEvent(void);
extern void lib_osCreateThread(void);
extern void lib_osRecvMesg(void);
extern void lib_osSpTaskLoad(void);
extern void lib_osSpTaskStartGo(void);
extern void lib_osSpTaskYield(void);
extern void lib_osSendMesg(void);
extern void lib_osSpTaskYielded(void);
extern void lib_osStartThread(void);
extern void lib_osWritebackDCacheAll(void);
extern void lib_osCreateViManager(void);
extern void lib_osViSetMode(void);
extern void lib_osViBlack(void);
extern void lib_osViSetSpecialFeatures(void);
extern void lib_osCreatePiManager(void);
extern void lib_osSetThreadPri(void);
extern void lib_osInitialize(void);
extern void lib_osViSwapBuffer(void);
extern void lib_sqrtf(void);
extern void lib_osContStartReadData(void);
extern void lib_osContGetReadData(void);
extern void lib_osContInit(void);
extern void lib_osEepromProbe(void);
extern void lib___ull_div(void);
extern void lib___ll_lshift(void);
extern void lib___ll_mul(void);
extern void lib_osInvalDCache(void);
extern void lib_osPiStartDma(void);
extern void lib_bzero(void);
extern void lib_osInvalICache(void);
extern void lib_osEepromLongRead(void);
extern void lib_osEepromLongWrite(void);
extern void lib_bcopy(void);
extern void lib_guOrtho(void);
extern void lib_guPerspective(void);
extern void lib_osGetTime(void);
extern void lib___d_to_ull(void);
extern void lib___ull_to_d(void);
extern void lib_cosf(void);
extern void lib_sinf(void);
extern void lib_guTranslate(void);
extern void lib_guRotate(void);
extern void lib_guScale(void);
extern void lib_osAiSetFrequency(void);
extern void lib_alSeqFileNew(void);
extern void lib_osWritebackDCache(void);
extern void lib_osAiGetLength(void);
extern void lib_osAiSetNextBuffer(void);
extern void lib_osVirtualToPhysical(void);
#endif

#ifdef APP_C3
extern void lib_osInitialize(void);
extern void lib_osAiGetLength(void);
extern void lib_osAiSetFrequency(void);
extern void lib_osAiSetNextBuffer(void);
extern void lib_osInvalDCache(void);
extern void lib_osInvalICache(void);
extern void lib_osWritebackDCacheAll(void);
extern void lib_osContStartReadData(void);
extern void lib_osContGetReadData(void);
extern void lib_osContInit(void);
extern void lib_osVirtualToPhysical(void);
extern void lib_sqrtf(void);
extern void lib_cosf(void);
extern void lib_guOrtho(void);
extern void lib_guPerspective(void);
extern void lib_sinf(void);
extern void lib_bcopy(void);
extern void lib_bzero(void);
extern void lib___ull_div(void);
extern void lib___ll_lshift(void);
extern void lib___ll_mul(void);
extern void lib_80304094(void);
extern void lib_803040C0(void);
extern void lib_sprintf(void);
extern void lib_80304B38(void);
extern void lib_osCreateMesgQueue(void);
extern void lib_osRecvMesg(void);
extern void lib_osSendMesg(void);
extern void lib_osSetEventMesg(void);
extern void lib_80304F80(void);
extern void lib_osSpTaskLoad(void);
extern void lib_osSpTaskStartGo(void);
extern void lib_osSpTaskYield(void);
extern void lib_osSpTaskYielded(void);
extern void lib_osCreateThread(void);
extern void lib_osSetThreadPri(void);
extern void lib_osStartThread(void);
extern void lib___osGetCurrFaultedThread(void);
extern void lib_osGetTime(void);
extern void lib_osSetTime(void);
extern void lib_osMapTLB(void);
extern void lib_osUnmapTLBAll(void);
extern void lib_osCreateViManager(void);
extern void lib_osViSetEvent(void);
extern void lib_osViSetMode(void);
extern void lib_osViSetSpecialFeatures(void);
extern void lib_osViSwapBuffer(void);
extern void lib_osViBlack(void);
extern void lib_guScale(void);
extern void lib_guTranslate(void);
extern void lib_guRotate(void);
extern void lib_osEepromProbe(void);
extern void lib_osEepromLongWrite(void);
extern void lib_osEepromLongRead(void);
extern void lib___osDisableInt(void);
extern void lib___osRestoreInt(void);
extern void lib_osCreatePiManager(void);
extern void lib_osEPiStartDma(void);
extern void lib_80308350(void);
extern void lib_osPiStartDma(void);
extern void lib_80308D10(void);
extern void lib_80308D18(void);
#endif

#ifdef APP_E4
extern void lib_80248AF0(void);
extern void lib_80278074(void);
extern void lib_80278974(void);
extern void lib_80278B98(void);
extern void lib_8027E490(void);
extern void lib_8027E520(void);
extern void lib_8027E5CC(void);
extern void lib_osCreateMesgQueue(void);
extern void lib_osSetEventMesg(void);
extern void lib_osViSetEvent(void);
extern void lib_osCreateThread(void);
extern void lib_osRecvMesg(void);
extern void lib_osSpTaskLoad(void);
extern void lib_osSpTaskStartGo(void);
extern void lib_osSpTaskYield(void);
extern void lib_osSendMesg(void);
extern void lib_osSpTaskYielded(void);
extern void lib_osStartThread(void);
extern void lib_osWritebackDCacheAll(void);
extern void lib_osCreateViManager(void);
extern void lib_osViSetMode(void);
extern void lib_osViBlack(void);
extern void lib_osViSetSpecialFeatures(void);
extern void lib_osCreatePiManager(void);
extern void lib_osSetThreadPri(void);
extern void lib_osInitialize(void);
extern void lib_sqrtf(void);
extern void lib_osInvalDCache(void);
extern void lib_osPiStartDma(void);
extern void lib_osAiSetFrequency(void);
extern void lib_alSeqFileNew(void);
extern void lib_osWritebackDCache(void);
extern void lib_osAiGetLength(void);
extern void lib_osAiSetNextBuffer(void);
#endif
#endif

#ifdef APP_UNK4
extern void lib_osSendMesg(void);
extern void lib_osStopThread(void);
extern void lib_osRecvMesg(void);
extern void lib_osSetIntMask(void);
extern void lib_sinf(void);
extern void lib_osSpTaskLoad(void);
extern void lib_osSpTaskStartGo(void);
extern void lib_osDestroyThread(void);
extern void lib___ull_div(void);
extern void lib___ll_mul(void);
extern void lib_bzero(void);
extern void lib___osMotorAccess(void);
extern void lib_osMotorInit(void);
extern void lib_osContReset(void);
extern void lib_osEepromWrite(void);
extern void lib_osCreateThread(void);
extern void lib_osContStartReadData(void);
extern void lib_osContGetReadData(void);
extern void lib_osEepromLongRead(void);
extern void lib_osVirtualToPhysical(void);
extern void lib_osWritebackDCache(void);
extern void lib_osInitialize(void);
extern void lib_osViGetNextFramebuffer(void);
extern void lib_osEPiLinkHandle(void);
extern void lib_osViBlack(void);
extern void lib_osSpTaskYield(void);
extern void lib_80030794(void);
extern void lib_guMtxIdentF(void);
extern void lib_osViSetMode(void);
extern void lib_osPfsAllocateFile(void);
extern void lib_osGetCount(void);
extern void lib_osEepromProbe(void);
extern void lib_osPfsFindFile(void);
extern void lib_osCreatePiManager(void);
extern void lib_osSetEventMesg(void);
extern void lib_sqrtf(void);
extern void lib_osAfterPreNMI(void);
extern void lib_osContStartQuery(void);
extern void lib_osContGetQuery(void);
extern void lib__Printf(void);
extern void lib_osEPiStartDma(void);
extern void lib_memcpy(void);
extern void lib_osCreateMesgQueue(void);
extern void lib_osInvalICache(void);
extern void lib_osInvalDCache(void);
extern void lib_osEepromLongWrite(void);
extern void lib_osSetThreadPri(void);
extern void lib_osGetThreadPri(void);
extern void lib_osViSwapBuffer(void);
extern void lib_guMtxXFMF(void);
extern void lib_guMtxCatF(void);
extern void lib_osSpTaskYielded(void);
extern void lib_osGetTime(void);
extern void lib_osAiSetFrequency(void);
extern void lib_guNormalize(void);
extern void lib___osGetActiveQueue(void);
extern void lib_alCopy(void);
extern void lib_osPfsDeleteFile(void);
extern void lib_cosf(void);
extern void lib_osSetTime(void);
extern void lib_osViSetEvent(void);
extern void lib_osCartRomInit(void);
extern void lib_guS2DInitBg(void);
extern void lib_80035D30(void);
extern void lib_alCents2Ratio(void);
extern void lib_osDpSetNextBuffer(void);
extern void lib_osCreateViManager(void);
extern void lib_osWritebackDCacheAll(void);
extern void lib_osStartThread(void);
extern void lib_osViSetYScale(void);
extern void lib_osAiSetNextBuffer(void);
extern void lib_osEepromRead(void);
extern void lib_osViGetCurrentFramebuffer(void);
/* ext */
extern void lib_osAiGetLength(void);
#endif

#endif /* __ASSEMBLER__ */

#endif /* _LIB_H_ */
