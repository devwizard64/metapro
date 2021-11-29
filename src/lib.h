#ifndef __LIB_H__
#define __LIB_H__

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

#define mtxf_ortho_bg(mf, l, r, b, t, n, f)                         \
{                                                                   \
    float _y = (1.0F/2) * ((t)+(b));                                \
    float _h = (1.0F/2) * ((t)-(b)) / ((3.0F/4.0F)*video_aspect);   \
    mtxf_ortho(mf, l, r, _y-_h, _y+_h, n, f);                       \
}

#define mtxf_ortho_fg(mf, l, r, b, t, n, f)                         \
{                                                                   \
    float _x = (1.0F/2) * ((r)+(l));                                \
    float _w = (1.0F/2) * ((r)-(l)) * ((3.0F/4.0F)*video_aspect);   \
    mtxf_ortho(mf, _x-_w, _x+_w, b, t, n, f);                       \
}

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

extern void mtx_read(f32 *dst, const s16 *src);
extern void mtx_write(s16 *dst, const f32 *src);
extern void mtxf_cat(f32 mf[4][4], f32 a[4][4], f32 b[4][4]);
extern void mtxf_identity(f32 mf[4][4]);
extern void mtxf_ortho(
    f32 mf[4][4], float l, float r, float b, float t, float n, float f
);

extern void thread_yield(int arg);
extern void thread_fault(void);

extern void lib_main(void (*start)(void));
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

#ifdef APP_UNKT
#ifdef APP_E0
extern void lib_osCreateThread(void);
extern void lib_osInitialize(void);
extern void lib_osStartThread(void);
extern void lib_osCreateViManager(void);
extern void lib_osViSetMode(void);
extern void lib_osViBlack(void);
extern void lib_osViSetSpecialFeatures(void);
extern void lib_osCreatePiManager(void);
extern void lib_osSetThreadPri(void);
extern void lib_osCreateMesgQueue(void);
extern void lib_osViSetEvent(void);
extern void lib_osSetEventMesg(void);
extern void lib_osSpTaskLoad(void);
extern void lib_osSpTaskStartGo(void);
extern void lib_osContInit(void);
extern void lib_osContStartReadData(void);
extern void lib_osContGetReadData(void);
extern void lib_osRecvMesg(void);
extern void lib_osWritebackDCacheAll(void);
extern void lib_osSendMesg(void);
extern void lib_osViSwapBuffer(void);
extern void lib_bzero(void);
extern void lib_osInvalICache(void);
extern void lib_osInvalDCache(void);
extern void lib_osPiStartDma(void);
extern void lib_osSpTaskYield(void);
extern void lib_osSpTaskYielded(void);
extern void lib_osGetTime(void);
extern void lib___ull_rem(void);
extern void lib___ull_div(void);
extern void lib___ll_div(void);
extern void lib___ll_mul(void);
extern void lib___osGetCurrFaultedThread(void);
extern void lib_sqrtf(void);
extern void lib_guOrtho(void);
extern void lib_osSetTime(void);
extern void lib_osEepromProbe(void);
extern void lib_osPfsIsPlug(void);
extern void lib_osPfsInitPak(void);
extern void lib_osPfsNumFiles(void);
extern void lib_osPfsFileState(void);
extern void lib_osPfsFreeBlocks(void);
extern void lib_guRotate(void);
extern void lib_guScale(void);
extern void lib_guPerspective(void);
extern void lib_guLookAtF(void);
extern void lib_guLookAt(void);
extern void lib_guTranslate(void);
extern void lib_osSyncPrintf(void);
extern void lib_guMtxCatL(void);
extern void lib_osPfsFindFile(void);
extern void lib_osPfsDeleteFile(void);
extern void lib_osEepromLongWrite(void);
extern void lib_osEepromLongRead(void);
extern void lib_osPfsReadWriteFile(void);
extern void lib_osPfsAllocateFile(void);
extern void lib_osAiSetFrequency(void);
extern void lib_osAiGetLength(void);
extern void lib_osAiSetNextBuffer(void);
extern void lib_osGetCount(void);
extern void lib_osWritebackDCache(void);
extern void lib_bcopy(void);
extern void lib_osVirtualToPhysical(void);
extern void lib_osSetTimer(void);
extern void lib_sinf(void);
extern void lib_cosf(void);
extern void lib_guMtxCatF(void);
#endif
#endif

#ifdef APP_UNK4
#ifdef APP_E0
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
#endif

#endif /* __ASSEMBLER__ */

#endif /* __LIB_H__ */
