#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _NATIVE
#include <time.h>
#endif
#ifndef _NATIVE
#include <malloc.h>
#endif
#include <setjmp.h>

#include <math.h>
#ifdef _NATIVE
#include <SDL2/SDL.h>
#endif
#ifndef _GCN
#include <GL/gl.h>
#endif

#include "types.h"
#include "cpu.h"
#include "gsp.h"
#include "asp.h"
#include "lib.h"
#include "app.h"

#include "ultra64.h"

#define VIDEO_SCALE             4

#ifdef _3DS
#define CONFIG_FLAG_STICK_X     0x01
#define CONFIG_FLAG_STICK_Y     0x02
#define CONFIG_FLAG_AUDIO       0x04
#define CONFIG_FLAG_HI_H        0x08
#define CONFIG_FLAG_AA_H        0x10
#define CONFIG_FLAG_AA_V        0x20
#endif
#ifdef _GCN
#define CONFIG_FLAG_AUDIO       0x01
#endif

#define THREAD_STACK_SIZE       0x4000

struct thread_t
{
    struct thread_t *lprev;
    struct thread_t *lnext;
    struct thread_t *qprev;
    struct thread_t *qnext;
    jmp_buf jmp;
    u8     *stack;
    u8      init;
    u8      ready;
    u8      qlink;
    u32     addr;
    u32     entry;
    s32     id;
    s32     pri;
    reg_t   reg[lenof(cpu_reg)];
};

struct os_event_t
{
    u32 mq;
    u32 msg;
};

struct os_pad_t
{
    u16 button;
    s8  stick_x;
    s8  stick_y;
};

struct config_t
{
#ifdef _NATIVE
    struct
    {
        u8 id;
        s8 mul;
    }
    input[16];
#endif
#ifdef _3DS
    u32 input[20];
    u32 input_exit;
    u32 input_save;
    u32 input_load;
    u32 flag;
#endif
#ifdef _GCN
    u16 input[20];
    u16 input_exit;
    u16 input_save;
    u16 input_load;
    u16 flag;
#endif
};

#ifndef APP_SEQ
static const struct config_t lib_config_default =
{
#ifdef _NATIVE
    {
        { 0,   0}, { 3,   0}, { 4,   0}, { 7,   0},
        { 8,   0}, { 9,   0}, {10,   0}, {11,   0},
        { 0,  78}, { 1, -78}, { 6,   0}, { 5,   0},
        { 4, -78}, { 4,  78}, { 3, -78}, { 3,  78},
    },
#endif
#ifdef _3DS
    {
        KEY_A,
        KEY_B | KEY_X,
        KEY_L,
        KEY_START,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        KEY_R,
        KEY_DUP    | KEY_CSTICK_UP,
        KEY_DDOWN  | KEY_CSTICK_DOWN,
        KEY_DLEFT  | KEY_CSTICK_LEFT,
        KEY_DRIGHT | KEY_CSTICK_RIGHT,
        0,
        0,
        0,
        0,
    },
    KEY_SELECT,
    KEY_ZL,
    KEY_ZR,
    CONFIG_FLAG_STICK_X | CONFIG_FLAG_STICK_Y | CONFIG_FLAG_AUDIO,
#endif
#ifdef _GCN
    {
        PAD_BUTTON_A,
        PAD_BUTTON_B | PAD_BUTTON_Y,
        PAD_TRIGGER_L | PAD_TRIGGER_Z,
        PAD_BUTTON_START,
        0,
        0,
        0,
        0,
        0x8000 | (u8) 78,
        0x8100 | (u8) 78,
        0,
        PAD_TRIGGER_R,
        0x8300 | (u8) 78,
        0x8300 | (u8)-78,
        0x8200 | (u8)-78,
        0x8200 | (u8) 78,
        0,
        0,
        0,
        0,
    },
    PAD_BUTTON_DOWN,
    PAD_BUTTON_LEFT,
    PAD_BUTTON_RIGHT,
    CONFIG_FLAG_AUDIO,
#endif
};
#endif

#ifdef _NATIVE
#ifndef APP_SEQ
static SDL_Window       *lib_window       = NULL;
static SDL_GLContext    *lib_context      = NULL;
static SDL_Joystick     *lib_joystick     = NULL;
#endif
static SDL_AudioDeviceID lib_audio_device = 0;
static clock_t           lib_clock        = 0;
#endif
#ifdef _3DS
static Thread lib_gsp_thread = NULL;
static Thread lib_asp_thread = NULL;
static Handle lib_gsp_start  = 0;
static Handle lib_gsp_end    = 0;
static Handle lib_asp_start  = 0;
static Handle lib_asp_end    = 0;
static void  *lib_gsp_ucode  = NULL;
static u32   *lib_gsp_data   = NULL;
static u32   *lib_asp_data   = NULL;
static u32    lib_asp_size   = 0;
static u8     lib_rsp_update = 1;
static f32 lib_audio_mix[12] =
{
    1.0F, 1.0F, 0.0F, 0.0F,
    0.0F, 0.0F, 0.0F, 0.0F,
    0.0F, 0.0F, 0.0F, 0.0F,
};
static ndspWaveBuf lib_audio_bufs[2] = {0};
static u8          lib_audio_buf     = 0;
#endif
#ifdef _GCN
static GXRModeObj *lib_rmode       = NULL;
static void       *lib_framebuffer = NULL;
static u8          lib_video_draw  = 0;
#endif

#ifndef APP_SEQ
static struct config_t lib_config;

#ifdef _NATIVE
u16 lib_video_w = VIDEO_SCALE*320; /* 400 */
u16 lib_video_h = VIDEO_SCALE*240;
f32 lib_viewport_l =   0.0F; /* -40.0F */
f32 lib_viewport_r = 320.0F; /* 360.0F */
#else
u16 lib_video_w    = 320;
u16 lib_video_h    = 240;
f32 lib_viewport_l =   0.0F;
f32 lib_viewport_r = 320.0F;
#endif

#ifdef _NATIVE
#ifdef APP_UNK4
static u32 lib_frame = 0;
#endif
static u8 lib_fast = 0;
#endif
static u8 lib_save = 0;
static u8 lib_load = 0;
#endif

static struct thread_t *lib_thread_list  = NULL;
static struct thread_t *lib_thread_queue = NULL;
static struct thread_t *lib_thread       = NULL;
static jmp_buf lib_jmp;

static struct os_event_t lib_event_table[OS_NUM_EVENTS] = {0};
static struct os_event_t lib_vi_event = {0};

#ifndef APP_SEQ
static struct os_pad_t *lib_input_data = NULL;
static struct os_pad_t *lib_input      = NULL;
static size_t           lib_input_size = 0;
static struct os_pad_t  lib_pad        = {0};
#endif

#ifndef APP_SEQ
void mtx_write(s16 *dst, const f32 *src)
{
    uint cnt = 4*4;
    do
    {
        s32 a = 0x10000 * src[0x00];
        s32 b = 0x10000 * src[0x01];
    #ifdef _GCN
        dst[0x00] = a >> 16;
        dst[0x01] = b >> 16;
        dst[0x10] = a >>  0;
        dst[0x11] = b >>  0;
    #else
        dst[0x00] = b >> 16;
        dst[0x01] = a >> 16;
        dst[0x10] = b >>  0;
        dst[0x11] = a >>  0;
    #endif
        dst += 0x02;
        src += 0x02;
        cnt -= 0x02;
    }
    while (cnt > 0);
}

void mtxf_mul(f32 mtxf[4][4], f32 a[4][4], f32 b[4][4])
{
    uint y;
    for (y = 0; y < 4; y++)
    {
        uint x;
        for (x = 0; x < 4; x++)
        {
            mtxf[y][x] =
                a[y][0]*b[0][x] + a[y][1]*b[1][x] +
                a[y][2]*b[2][x] + a[y][3]*b[3][x];
        }
    }
}

void mtxf_identity(f32 mtxf[4][4])
{
    mtxf[0][0] = 1.0F;
    mtxf[0][1] = 0.0F;
    mtxf[0][2] = 0.0F;
    mtxf[0][3] = 0.0F;
    mtxf[1][0] = 0.0F;
    mtxf[1][1] = 1.0F;
    mtxf[1][2] = 0.0F;
    mtxf[1][3] = 0.0F;
    mtxf[2][0] = 0.0F;
    mtxf[2][1] = 0.0F;
    mtxf[2][2] = 1.0F;
    mtxf[2][3] = 0.0F;
    mtxf[3][0] = 0.0F;
    mtxf[3][1] = 0.0F;
    mtxf[3][2] = 0.0F;
    mtxf[3][3] = 1.0F;
}

void mtxf_ortho(f32 mtxf[4][4], f32 l, f32 r, f32 b, f32 t, f32 n, f32 f)
{
    mtxf[0][0] = 2.0F / (r-l);
    mtxf[0][1] = 0.0F;
    mtxf[0][2] = 0.0F;
    mtxf[0][3] = 0.0F;
    mtxf[1][0] = 0.0F;
    mtxf[1][1] = 2.0F / (t-b);
    mtxf[1][2] = 0.0F;
    mtxf[1][3] = 0.0F;
    mtxf[2][0] = 0.0F;
    mtxf[2][1] = 0.0F;
    mtxf[2][2] = 2.0F / (n-f);
    mtxf[2][3] = 0.0F;
    mtxf[3][0] = (l+r) / (l-r);
    mtxf[3][1] = (b+t) / (b-t);
    mtxf[3][2] = (n+f) / (n-f);
    mtxf[3][3] = 1.0F;
}

#ifndef APP_UNK4
static void mtxf_perspective(f32 mtxf[4][4], f32 fovy, f32 aspect, f32 n, f32 f)
{
    f32 x;
    fovy *= (f32)(M_PI / 360.0);
    x = cosf(fovy) / sinf(fovy);
    mtxf[0][0] = x/aspect;
    mtxf[0][1] = 0.0F;
    mtxf[0][2] = 0.0F;
    mtxf[0][3] = 0.0F;
    mtxf[1][0] = 0.0F;
    mtxf[1][1] = x;
    mtxf[1][2] = 0.0F;
    mtxf[1][3] = 0.0F;
    x = 1.0F / (n-f);
    mtxf[2][0] = 0.0F;
    mtxf[2][1] = 0.0F;
#ifdef _GCN
    mtxf[2][2] = (n  )*x;
#else
    mtxf[2][2] = (n+f)*x;
#endif
    mtxf[2][3] = -1.0F;
    mtxf[3][0] = 0.0F;
    mtxf[3][1] = 0.0F;
#ifdef _GCN
    mtxf[3][2] =      n*f*x;
#else
    mtxf[3][2] = 2.0F*n*f*x;
#endif
    mtxf[3][3] = 0.0F;
}

static void mtxf_translate(f32 mtxf[4][4], f32 x, f32 y, f32 z)
{
    mtxf[0][0] = 1.0F;
    mtxf[0][1] = 0.0F;
    mtxf[0][2] = 0.0F;
    mtxf[0][3] = 0.0F;
    mtxf[1][0] = 0.0F;
    mtxf[1][1] = 1.0F;
    mtxf[1][2] = 0.0F;
    mtxf[1][3] = 0.0F;
    mtxf[2][0] = 0.0F;
    mtxf[2][1] = 0.0F;
    mtxf[2][2] = 1.0F;
    mtxf[2][3] = 0.0F;
    mtxf[3][0] = x;
    mtxf[3][1] = y;
    mtxf[3][2] = z;
    mtxf[3][3] = 1.0F;
}

static void mtxf_rotate(f32 mtxf[4][4], f32 a, f32 x, f32 y, f32 z)
{
    f32 xx;
    f32 yy;
    f32 zz;
    f32 s;
    f32 c;
    f32 xyc;
    f32 yzc;
    f32 zxc;
    xx = x*x;
    yy = y*y;
    zz = z*z;
    s = 1.0F / sqrtf(xx + yy + zz);
    x *= s;
    y *= s;
    z *= s;
    a *= (f32)(M_PI / 180.0);
    s = sinf(a);
    c = cosf(a);
    xyc = x*y*(1.0F-c);
    yzc = y*z*(1.0F-c);
    zxc = z*x*(1.0F-c);
    mtxf[0][0] = (1.0F-xx)*c + xx;
    mtxf[0][1] = xyc + z*s;
    mtxf[0][2] = zxc - y*s;
    mtxf[0][3] = 0.0F;
    mtxf[1][0] = xyc - z*s;
    mtxf[1][1] = (1.0F-yy)*c + yy;
    mtxf[1][2] = yzc + x*s;
    mtxf[1][3] = 0.0F;
    mtxf[2][0] = zxc + y*s;
    mtxf[2][1] = yzc - x*s;
    mtxf[2][2] = (1.0F-zz)*c + zz;
    mtxf[2][3] = 0.0F;
    mtxf[3][0] = 0.0F;
    mtxf[3][1] = 0.0F;
    mtxf[3][2] = 0.0F;
    mtxf[3][3] = 1.0F;
}

static void mtxf_scale(f32 mtxf[4][4], f32 x, f32 y, f32 z)
{
    mtxf[0][0] = x;
    mtxf[0][1] = 0.0F;
    mtxf[0][2] = 0.0F;
    mtxf[0][3] = 0.0F;
    mtxf[1][0] = 0.0F;
    mtxf[1][1] = y;
    mtxf[1][2] = 0.0F;
    mtxf[1][3] = 0.0F;
    mtxf[2][0] = 0.0F;
    mtxf[2][1] = 0.0F;
    mtxf[2][2] = z;
    mtxf[2][3] = 0.0F;
    mtxf[3][0] = 0.0F;
    mtxf[3][1] = 0.0F;
    mtxf[3][2] = 0.0F;
    mtxf[3][3] = 1.0F;
}
#endif
#endif

static void thread_llink(struct thread_t *thread)
{
    struct thread_t **list = &lib_thread_list;
    struct thread_t  *prev = NULL;
    while (*list != NULL)
    {
        if (*list == thread)
        {
            return;
        }
        prev = *list;
        list = &prev->lnext;
    }
    *list = thread;
    thread->lprev = prev;
    thread->lnext = NULL;
}

static void thread_qlink(struct thread_t *thread)
{
    struct thread_t **queue = &lib_thread_queue;
    struct thread_t  *prev  = NULL;
    while (*queue != NULL)
    {
        if (*queue == thread)
        {
            return;
        }
        prev = *queue;
        queue = &prev->qnext;
    }
    *queue = thread;
    thread->qprev = prev;
    thread->qnext = NULL;
    thread->qlink = 1;
}

static void thread_lunlink(struct thread_t *thread)
{
    if (thread->lprev != NULL)
    {
        thread->lprev->lnext = thread->lnext;
    }
    else
    {
        lib_thread_list = thread->lnext;
    }
    if (thread->lnext != NULL)
    {
        thread->lnext->lprev = thread->lprev;
    }
}

static void thread_qunlink(struct thread_t *thread)
{
    if (thread->qlink)
    {
        thread->qlink = 0;
        if (thread->qprev != NULL)
        {
            thread->qprev->qnext = thread->qnext;
        }
        else
        {
            lib_thread_queue = thread->qnext;
        }
        if (thread->qnext != NULL)
        {
            thread->qnext->qprev = thread->qprev;
        }
    }
}

void thread_yield(int arg)
{
    if (lib_thread == NULL || setjmp(lib_thread->jmp) == 0)
    {
        longjmp(lib_jmp, arg);
    }
}

static struct thread_t *thread_find(u32 addr)
{
    struct thread_t *thread;
    if (addr == 0)
    {
        thread = lib_thread;
    }
    else
    {
        thread = lib_thread_list;
        while (thread != NULL && thread->addr != addr)
        {
            thread = thread->lnext;
        }
    }
    return thread;
}

/*
static void thread_print(void)
{
    struct thread_t *queue = lib_thread_queue;
    while (queue != NULL)
    {
        printf(
            "entry=%08X  pri=%3d  r=%d  id=%d\n",
            queue->entry, queue->pri, queue->ready, queue->id
        );
        queue = queue->qnext;
    }
}
*/

s32 thread_id(void)
{
    return lib_thread != NULL ? lib_thread->id : 0;
}

static void thread_init(u32 addr, s32 id, u32 entry, s32 arg, s32 s, u32 pri)
{
    struct thread_t *thread = malloc(sizeof(*thread));
    thread_llink(thread);
#ifdef _NATIVE
    thread->stack        = malloc(THREAD_STACK_SIZE);
#else
    thread->stack        = memalign(0x20, THREAD_STACK_SIZE);
#endif
    thread->init         = 1;
    thread->ready        = 1;
    thread->qlink        = 0;
    thread->addr         = addr;
    thread->entry        = entry;
    thread->id           = id;
    thread->pri          = pri;
    thread->reg[R_A0].ll = arg;
    thread->reg[R_SP].ll = s - 0x10;
}

static void thread_start(struct thread_t *thread)
{
    thread_qlink(thread);
    thread_yield(THREAD_YIELD_QUEUE);
}

#ifdef APP_UNK4
static void thread_stop(struct thread_t *thread)
{
    thread_qunlink(thread);
    thread_yield(THREAD_YIELD_QUEUE);
}
#endif

static void thread_destroy(struct thread_t *thread)
{
    thread_lunlink(thread);
    thread_qunlink(thread);
    free(thread->stack);
    free(thread);
    if (thread == lib_thread)
    {
        longjmp(lib_jmp, THREAD_YIELD_QUEUE);
    }
}

void thread_fault(void)
{
    if (lib_thread != NULL)
    {
        thread_destroy(lib_thread);
    }
    else
    {
        exit(EXIT_FAILURE);
    }
}

static void lib_event(struct os_event_t *event)
{
    if (event->mq != 0)
    {
        a0.i[IX] = event->mq;
        a1.i[IX] = event->msg;
        a2.i[IX] = OS_MESG_NOBLOCK;
        lib_osSendMesg();
    }
}

#ifndef APP_SEQ
static void video_update_size(s32 w, s32 h)
{
    f32 x;
    lib_video_w = w;
    lib_video_h = h;
    x = 120.0F * w/h;
    lib_viewport_l = 160.0F - x;
    lib_viewport_r = 160.0F + x;
}
#endif

#ifdef _3DS
static void gsp_main(unused void *arg)
{
    gsp_init();
    do
    {
        svcSignalEvent(lib_gsp_end);
        svcWaitSynchronization(lib_gsp_start, U64_MAX);
        svcClearEvent(lib_gsp_start);
        if (lib_gsp_data != NULL)
        {
            gsp_update(lib_gsp_ucode, lib_gsp_data);
        }
    }
    while (lib_rsp_update);
    gsp_destroy();
}

static void asp_main(unused void *arg)
{
    do
    {
        svcSignalEvent(lib_asp_end);
        svcWaitSynchronization(lib_asp_start, U64_MAX);
        svcClearEvent(lib_asp_start);
        if (lib_asp_data != NULL)
        {
            asp_update(lib_asp_data, lib_asp_size);
        }
    }
    while (lib_rsp_update);
}
#endif

#ifdef _GCN
static void video_draw(unused u32 count)
{
    if (lib_video_draw)
    {
        lib_video_draw = 0;
        GX_SetZMode(GX_TRUE, GX_ALWAYS, GX_TRUE);
        GX_SetColorUpdate(GX_TRUE);
        GX_CopyDisp(lib_framebuffer, GX_TRUE);
        GX_Flush();
    }
}
#endif

#ifndef APP_SEQ
static void video_init(void)
{
#ifdef _NATIVE
    SDL_SetHint(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS, "1");
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER,       1);
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE,           8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE,         8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,          8);
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
    lib_window = SDL_CreateWindow(
        "app", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, lib_video_w,
        lib_video_h, SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL
    );
    if (lib_window == NULL)
    {
        fprintf(
            stderr, "error: could not create window (%s)\n", SDL_GetError()
        );
        exit(EXIT_FAILURE);
    }
    lib_context = SDL_GL_CreateContext(lib_window);
    if (lib_context == NULL)
    {
        fprintf(
            stderr, "error: could not create context (%s)\n", SDL_GetError()
        );
        exit(EXIT_FAILURE);
    }
    SDL_GL_SetSwapInterval(1);
    gsp_init();
#endif
#ifdef _3DS
    gfxInitDefault();
    if (lib_config.flag & CONFIG_FLAG_HI_H)
    {
        gfxSetWide(1);
        video_update_size(800, 240);
    }
    else
    {
        gfxSet3D(1);
        video_update_size(400, 240);
    }
    consoleInit(GFX_BOTTOM, NULL);
    svcCreateEvent(&lib_gsp_start, RESET_ONESHOT);
    svcCreateEvent(&lib_gsp_end,   RESET_ONESHOT);
    svcCreateEvent(&lib_asp_start, RESET_ONESHOT);
    svcCreateEvent(&lib_asp_end,   RESET_ONESHOT);
    lib_gsp_thread = threadCreate(gsp_main, NULL, 0x2000, 0x3F, -1, 1);
    lib_asp_thread = threadCreate(asp_main, NULL, 0x2000, 0x3F, -1, 1);
#endif
#ifdef _GCN
    void *fifo;
    VIDEO_Init();
    lib_rmode = VIDEO_GetPreferredMode(NULL);
    VIDEO_Configure(lib_rmode);
    lib_framebuffer = MEM_K0_TO_K1(SYS_AllocateFramebuffer(lib_rmode));
    VIDEO_SetNextFramebuffer(lib_framebuffer);
    VIDEO_SetPostRetraceCallback(video_draw);
    VIDEO_SetBlack(0);
    VIDEO_Flush();
    fifo = MEM_K0_TO_K1(memalign(0x20, 0x40000));
    GX_Init(fifo, 0x40000);
    GX_SetCopyClear((GXColor){0x00, 0x00, 0x00, 0xFF}, GX_MAX_Z24);
    GX_SetDispCopyYScale((f32)lib_rmode->xfbHeight / (f32)lib_rmode->efbHeight);
    GX_SetDispCopySrc(0, 0, lib_rmode->fbWidth, lib_rmode->efbHeight);
    GX_SetDispCopyDst(lib_rmode->fbWidth, lib_rmode->xfbHeight);
    GX_SetCopyFilter(
        lib_rmode->aa, lib_rmode->sample_pattern, GX_TRUE, lib_rmode->vfilter
    );
    GX_SetFieldMode(
        lib_rmode->field_rendering,
        lib_rmode->viHeight == 2*lib_rmode->xfbHeight
    );
    GX_SetDispCopyGamma(GX_GM_1_0);
    GX_SetColorUpdate(GX_TRUE);
    GX_CopyDisp(lib_framebuffer, GX_TRUE);
    video_update_size(lib_rmode->fbWidth, lib_rmode->efbHeight);
    gsp_init();
#endif
}
#endif

#ifndef APP_SEQ
static void video_destroy(void)
{
#ifdef _NATIVE
    if (lib_window != NULL)
    {
        SDL_DestroyWindow(lib_window);
    }
#endif
#ifdef _3DS
    lib_rsp_update = 0;
    if (lib_gsp_thread != NULL)
    {
        lib_gsp_data = NULL;
        svcSignalEvent(lib_gsp_start);
        threadJoin(lib_gsp_thread, U64_MAX);
        svcCloseHandle(lib_gsp_start);
    }
    if (lib_asp_thread != NULL)
    {
        lib_asp_data = NULL;
        svcSignalEvent(lib_asp_start);
        threadJoin(lib_asp_thread, U64_MAX);
        svcCloseHandle(lib_asp_start);
    }
    gfxExit();
#endif
}
#endif

void video_update(void)
{
#ifdef _NATIVE
#ifndef APP_SEQ
#ifdef APP_UNK4
    if (lib_frame++ == 60*53)
    {
        lib_fast = 0;
    }
#endif
    if (!lib_fast)
#endif
    {
        clock_t time = clock();
        s32 delay;
        if (lib_clock == 0)
        {
            lib_clock = time;
        }
        lib_clock += 16667;
        delay = (lib_clock-clock()+500) / 1000;
        if (delay > 0)
        {
            lib_clock -= 1000*delay;
            SDL_Delay(delay);
        }
    }
#endif
#ifdef _3DS
    gspWaitForVBlank();
#endif
#ifdef _GCN
    VIDEO_WaitVSync();
#endif
}

#ifndef APP_SEQ
static void input_init(void)
{
    FILE *f;
#ifdef _GCN
    PAD_Init();
#endif
    f = fopen(PATH_INPUT, "rb");
    if (f != NULL)
    {
        fseek(f, 0, SEEK_END);
        lib_input_size = ftell(f);
        fseek(f, 0, SEEK_SET);
        lib_input_data = lib_input = malloc(lib_input_size);
        fread(lib_input_data, 1, lib_input_size, f);
        fclose(f);
    }
#ifdef _NATIVE
    lib_joystick = SDL_JoystickOpen(0);
#endif
}
#endif

#ifndef APP_SEQ
static void input_destroy(void)
{
#ifdef _NATIVE
    if (lib_joystick != NULL)
    {
        SDL_JoystickClose(lib_joystick);
    }
#endif
}
#endif

#ifndef APP_SEQ
static void input_update(void)
{
#ifdef _NATIVE
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
            case SDL_QUIT:
                exit(EXIT_SUCCESS);
                break;
        #ifndef APP_SEQ
            case SDL_WINDOWEVENT:
                switch (event.window.event)
                {
                    case SDL_WINDOWEVENT_RESIZED:
                        video_update_size(
                            event.window.data1, event.window.data2
                        );
                        break;
                }
                break;
            case SDL_KEYDOWN:
                switch (event.key.keysym.scancode)
                {
                    case SDL_SCANCODE_F4:
                        lib_fast ^= 1;
                        break;
                    case SDL_SCANCODE_F5:
                        lib_save = 1;
                        break;
                    case SDL_SCANCODE_F7:
                        lib_load = 1;
                        break;
                    default:
                        break;
                }
                break;
        #endif
        }
    }
#else
#ifdef _3DS
    u32 held;
    u32 down;
    if (!aptMainLoop())
    {
        exit(EXIT_SUCCESS);
    }
    hidScanInput();
    held = hidKeysHeld();
    down = hidKeysDown();
#endif
#ifdef _GCN
    uint held;
    uint down;
    PAD_ScanPads();
    held = PAD_ButtonsHeld(0);
    down = PAD_ButtonsDown(0);
#endif
#define INPUT_COMBO(d, h, m) (((d) & (m)) && ((h) & (m)) == (m))
    if (INPUT_COMBO(down, held, lib_config.input_exit))
    {
        exit(EXIT_SUCCESS);
    }
    if (INPUT_COMBO(down, held, lib_config.input_save))
    {
        lib_save = 1;
    }
    if (INPUT_COMBO(down, held, lib_config.input_load))
    {
        lib_load = 1;
    }
#undef INPUT_COMBO
#endif
    if (lib_input_size > 0)
    {
    #ifdef _GCN
        lib_pad.button  = lib_input->button;
    #else
        lib_pad.button  = lib_input->button >> 8 | lib_input->button << 8;
    #endif
        lib_pad.stick_x = lib_input->stick_x;
        lib_pad.stick_y = lib_input->stick_y;
        lib_input++;
        lib_input_size -= sizeof(*lib_input);
        if (lib_input_size == 0)
        {
            free(lib_input_data);
        }
    }
    else
    {
    #ifdef _3DS
        circlePosition stick;
    #endif
        uint mask;
        uint i;
    #ifdef _3DS
        hidCircleRead(&stick);
    #endif
        lib_pad.button  = 0;
        lib_pad.stick_x = 0;
        lib_pad.stick_y = 0;
    #ifdef _NATIVE
        if (lib_joystick != NULL)
        {
            for (mask = 0x8000, i = 0; i < 16; i++, mask >>= 1)
            {
                uint id  = lib_config.input[i].id;
                int  mul = lib_config.input[i].mul;
                if (id != 0xFF)
                {
                    if (mul != 0)
                    {
                        s32 axis =
                            SDL_JoystickGetAxis(lib_joystick, id) *
                            mul/(0x100*100);
                        switch (mask)
                        {
                            case 0x0080:
                                lib_pad.stick_x = axis;
                                break;
                            case 0x0040:
                                lib_pad.stick_y = axis;
                                break;
                            default:
                                if (axis > 40)
                                {
                                    lib_pad.button |= mask;
                                }
                                break;
                        }
                    }
                    else
                    {
                        if (SDL_JoystickGetButton(lib_joystick, id))
                        {
                            lib_pad.button |= mask;
                        }
                    }
                }
            }
        }
        {
            const u8 *keys = SDL_GetKeyboardState(NULL);
            if (keys[SDL_SCANCODE_X])       lib_pad.button |= 0x8000;
            if (keys[SDL_SCANCODE_C])       lib_pad.button |= 0x4000;
            if (keys[SDL_SCANCODE_Z])       lib_pad.button |= 0x2000;
            if (keys[SDL_SCANCODE_RETURN])  lib_pad.button |= 0x1000;
            if (keys[SDL_SCANCODE_LCTRL])   lib_pad.button |= 0x0010;
            if (keys[SDL_SCANCODE_W])       lib_pad.button |= 0x0008;
            if (keys[SDL_SCANCODE_S])       lib_pad.button |= 0x0004;
            if (keys[SDL_SCANCODE_A])       lib_pad.button |= 0x0002;
            if (keys[SDL_SCANCODE_D])       lib_pad.button |= 0x0001;
            if (keys[SDL_SCANCODE_LEFT])
            {
                if (!keys[SDL_SCANCODE_RIGHT])
                {
                    lib_pad.stick_x = -80;
                }
            }
            else if (keys[SDL_SCANCODE_RIGHT])
            {
                lib_pad.stick_x = 80;
            }
            if (keys[SDL_SCANCODE_DOWN])
            {
                if (!keys[SDL_SCANCODE_UP])
                {
                    lib_pad.stick_y = -80;
                }
            }
            else if (keys[SDL_SCANCODE_UP])
            {
                lib_pad.stick_y = 80;
            }
        }
    #else
    #ifdef _3DS
        if (lib_config.flag & CONFIG_FLAG_STICK_X)
        {
            lib_pad.stick_x = stick.dx/2;
        }
        if (lib_config.flag & CONFIG_FLAG_STICK_Y)
        {
            lib_pad.stick_y = stick.dy/2;
        }
    #endif
        for (mask = 0x8000, i = 0; i < 20; i++, mask >>= 1)
        {
        #ifdef _GCN
            if (lib_config.input[i] & 0x8000)
            {
                s32 axis;
                switch (lib_config.input[i] >> 8)
                {
                    case 0x80: axis = PAD_StickX(0);    break;
                    case 0x81: axis = PAD_StickY(0);    break;
                    case 0x82: axis = PAD_SubStickX(0); break;
                    case 0x83: axis = PAD_SubStickY(0); break;
                }
                axis = axis*(s8)lib_config.input[i]/(0x100*100);
                switch (mask)
                {
                    case 0x0080:
                        lib_pad.stick_x = axis;
                        break;
                    case 0x0040:
                        lib_pad.stick_y = axis;
                        break;
                    default:
                        if (axis > 40)
                        {
                            lib_pad.button |= mask;
                        }
                        break;
                }
            }
            else
        #endif
            if (lib_config.input[i] & held)
            {
                switch (i)
                {
                    case 0x10:
                        lib_pad.stick_x = -80;
                        break;
                    case 0x11:
                        lib_pad.stick_x =  80;
                        break;
                    case 0x12:
                        lib_pad.stick_y = -80;
                        break;
                    case 0x13:
                        lib_pad.stick_y =  80;
                        break;
                    default:
                        lib_pad.button |= mask;
                        break;
                }
            }
        }
    #endif
    }
}
#endif

static s32 audio_size(void)
{
#ifdef _NATIVE
    return SDL_GetQueuedAudioSize(lib_audio_device);
#endif
#ifdef _3DS
    return 0x200;
#endif
#ifdef _GCN
    return AUDIO_GetDMABytesLeft();
#endif
}

static s32 audio_init(unused s32 freq)
{
#ifdef _NATIVE
    SDL_AudioSpec spec;
#ifdef APP_SEQ
    SDL_Init(SDL_INIT_AUDIO);
#endif
    spec.freq     = freq;
    spec.format   = AUDIO_S16;
    spec.channels = 2;
    spec.samples  = 0;
    spec.callback = NULL;
    spec.userdata = NULL;
    lib_audio_device = SDL_OpenAudioDevice(NULL, 0, &spec, NULL, 0);
    SDL_PauseAudioDevice(lib_audio_device, 0);
    return freq;
#endif
#ifdef _3DS
    ndspInit();
    ndspSetOutputMode(NDSP_OUTPUT_STEREO);
    ndspChnSetInterp(0, NDSP_INTERP_LINEAR);
    ndspChnSetRate(0, freq);
    ndspChnSetFormat(0, NDSP_FORMAT_STEREO_PCM16);
    ndspChnSetMix(0, lib_audio_mix);
    return freq;
#endif
#ifdef _GCN
    AUDIO_Init(NULL);
    AUDIO_SetDSPSampleRate(AI_SAMPLERATE_32KHZ);
    return 32000;
#endif
}

static void audio_destroy(void)
{
#ifdef _NATIVE
    SDL_CloseAudio();
#endif
#ifdef _3DS
    ndspExit();
#endif
#ifdef _GCN
    AUDIO_StopDMA();
#endif
}

static void audio_update(void *src, size_t size)
{
#ifdef _NATIVE
    void *data = malloc(size);
    __WORDSWAP(data, src, size);
#ifndef APP_SEQ
    if (lib_fast)
    {
        size /= 2;
    }
#endif
    SDL_QueueAudio(lib_audio_device, data, size);
    free(data);
#endif
#ifdef _3DS
    ndspWaveBuf *wb = &lib_audio_bufs[lib_audio_buf];
    if (wb->status == NDSP_WBUF_DONE || wb->status == NDSP_WBUF_FREE)
    {
        lib_audio_buf ^= 1;
        if (wb->data_pcm16 != NULL)
        {
            linearFree(wb->data_pcm16);
        }
        wb->data_pcm16 = linearAlloc(size);
        wb->nsamples = size / (2*sizeof(s16));
        __WORDSWAP(wb->data_pcm16, src, size);
        DSP_FlushDataCache(wb->data_pcm16, size);
        ndspChnWaveBufAdd(0, wb);
    }
#endif
#ifdef _GCN
    AUDIO_InitDMA((u32)src, size);
    AUDIO_StartDMA();
#endif
}

#ifndef APP_SEQ
static void config_init(void)
{
    FILE *f = fopen(PATH_CONFIG, "rb");
    if (f != NULL)
    {
        fread(&lib_config, 1, sizeof(lib_config), f);
        fclose(f);
    }
    else
    {
        memcpy(&lib_config, &lib_config_default, sizeof(lib_config));
        f = fopen(PATH_CONFIG, "wb");
        if (f != NULL)
        {
            fwrite(&lib_config, 1, sizeof(lib_config), f);
            fclose(f);
        }
        else
        {
        #ifdef _DEBUG
            fprintf(stderr, "warning: could not write '" PATH_CONFIG "'\n");
        #endif
        }
    }
}
#endif

static void lib_update(void)
{
#ifndef APP_SEQ
    if (lib_save)
    {
        FILE *f;
        lib_save = 0;
        f = fopen(PATH_DRAM, "wb");
        if (f != NULL)
        {
            fwrite(cpu_dram, 1, sizeof(cpu_dram), f);
            fclose(f);
        }
        else
        {
        #ifdef _DEBUG
            fprintf(
                stderr, "warning: could not write '" PATH_DRAM "'\n"
            );
        #endif
        }
    }
    if (lib_load)
    {
        FILE *f;
        lib_load = 0;
        f = fopen(PATH_DRAM, "rb");
        if (f != NULL)
        {
            fread(cpu_dram, 1, sizeof(cpu_dram), f);
            fclose(f);
            gsp_cache();
        }
        else
        {
        #ifdef _DEBUG
            fprintf(
                stderr, "warning: could not read '" PATH_DRAM "'\n"
            );
        #endif
        }
    }
#endif
    video_update();
    lib_event(&lib_vi_event);
    longjmp(lib_jmp, THREAD_YIELD_QUEUE);
}

static void lib_main(void)
{
    int arg = setjmp(lib_jmp);
    if (arg != THREAD_YIELD_NULL)
    {
        struct thread_t *queue;
        struct thread_t *thread;
        s32 pri;
        thread = lib_thread;
        lib_thread = NULL;
        if (thread != NULL)
        {
            memcpy(thread->reg, cpu_reg, sizeof(cpu_reg));
        }
        if (arg == THREAD_YIELD_BREAK)
        {
            lib_update();
        }
        queue = lib_thread_queue;
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
        lib_thread = thread;
        memcpy(cpu_reg, thread->reg, sizeof(cpu_reg));
        if (thread->init)
        {
            register void *stack;
            register u32   entry;
            thread->init = 0;
            stack = thread->stack + THREAD_STACK_SIZE;
            entry = thread->entry;
            asm volatile(
            #ifdef _NATIVE
                "mov %[stack], %%rsp"
            #endif
            #ifdef _3DS
                "mov sp, %[stack]"
            #endif
            #ifdef _GCN
                "mr 1, %[stack]"
            #endif
                : [stack] "+r" (stack) ::
            );
            __call(entry);
            thread_destroy(thread);
        }
        else
        {
            longjmp(thread->jmp, 1);
        }
    }
}

static void lib_destroy(void)
{
    cpu_destroy();
    audio_destroy();
#ifndef APP_SEQ
    input_destroy();
    video_destroy();
#endif
}

void lib_init(void)
{
    atexit(lib_destroy);
#ifndef APP_SEQ
    video_init();
    input_init();
    config_init();
#endif
    cpu_init();
    lib_main();
}

#ifndef APP_SEQ
void lib_cache(void)
{
    gsp_cache();
}
#endif

#define LIB_SV(f) void lib_##f(void) {}
#define LIB_S0(f) void lib_##f(void) {v0.ll = (s32)0;}

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
/* see osSpTaskStartGo */
LIB_SV(osSpTaskYield)
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

void lib___ull_div(void)
{
#ifdef _DEBUG
    puts("__ull_div");
    exit(EXIT_FAILURE);
#endif
}

#include "lib/__ll_lshift.c"

void lib___ll_mul(void)
{
#ifdef _DEBUG
    puts("__ll_mul");
    exit(EXIT_FAILURE);
#endif
}

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

void lib___d_to_ull(void)
{
#ifdef _DEBUG
    puts("__d_to_ull");
    exit(EXIT_FAILURE);
#endif
}

void lib___ull_to_d(void)
{
#ifdef _DEBUG
    puts("__ull_to_d");
    exit(EXIT_FAILURE);
#endif
}

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

void lib___ull_div(void)
{
#ifdef _DEBUG
    puts("__ull_div");
    exit(EXIT_FAILURE);
#endif
}

#include "lib/__ll_lshift.c"

void lib___ll_mul(void)
{
#ifdef _DEBUG
    puts("__ll_mul");
    exit(EXIT_FAILURE);
#endif
}

void lib_80304094(void)
{
    puts("80304094");
    exit(EXIT_FAILURE);
}

void lib_803040C0(void)
{
    puts("803040C0");
    exit(EXIT_FAILURE);
}

#include "lib/sprintf.c"
/* stub */
LIB_SV(80304B38)
#include "lib/osCreateMesgQueue.c"
#include "lib/osRecvMesg.c"
#include "lib/osSendMesg.c"
#include "lib/osSetEventMesg.c"

void lib_80304F80(void)
{
    /*
    puts("80304F80");
    exit(EXIT_FAILURE);
    */
}

LIB_SV(osSpTaskLoad)
#include "lib/osSpTaskStartGo.c"
/* see osSpTaskStartGo */
LIB_SV(osSpTaskYield)
LIB_S0(osSpTaskYielded)
#include "lib/osCreateThread.c"
#include "lib/osSetThreadPri.c"
#include "lib/osStartThread.c"

void lib___osGetCurrFaultedThread(void)
{
    puts("__osGetCurrFaultedThread");
    exit(EXIT_FAILURE);
}

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

void lib_80308350(void)
{
    /*
    puts("80308350");
    exit(EXIT_FAILURE);
    */
}

#include "lib/osPiStartDma.c"

void lib_80308D10(void)
{
    v0.ll = (s32)5;
}

void lib_80308D18(void)
{
    v0.ll = (s32)11;
}

/*
void lib___d_to_ull(void)
{
#ifdef _DEBUG
    puts("__d_to_ull");
    exit(EXIT_FAILURE);
#endif
}

void lib___ull_to_d(void)
{
#ifdef _DEBUG
    puts("__ull_to_d");
    exit(EXIT_FAILURE);
#endif
}

#include "lib/guTranslate.c"
#include "lib/guRotate.c"
#include "lib/guScale.c"
#include "lib/alSeqFileNew.c"
LIB_SV(osWritebackDCache)
*/

#endif

#ifdef APP_E4

struct meme_t
{
    u8  type;
    u32 id;
};

#define mq_app_vi   0x8033B010
#define msg_app_vi  0x8033B040
#define vq_app      0x8033B048
void lib_80248AF0(void)
{
    a0.i[IX] = mq_app_vi;
    a1.i[IX] = msg_app_vi;
    a2.i[IX] = 1;
    lib_osCreateMesgQueue();
    a0.i[IX] = 2;
    a1.i[IX] = vq_app;
    a2.i[IX] = mq_app_vi;
    a3.i[IX] = 1;
    app_80246B14();
    a0.i[IX] = 2;
    a1.i[IX] = 0;
    a2.i[IX] = 0;
    app_803219AC();
    a0.i[IX] = 0;
    app_80248E08();
    __write_u16(0x8032DDF8, 32);
    if (0 == 0)
    {
        a0.i[IX] = 0;
        a1.i[IX] = 0x0021;
        a2.i[IX] = 0x0000;
        app_803219AC();
    }
    else
    {
        __write_f32(0x80000400, 0);
        __write_f32(0x80000404, 0);
        __write_f32(0x80000408, 0);
        a0.i[IX] = 0x24228081;
        a1.i[IX] = 0x80000400;
        app_8031EB00();
    }
    while (1)
    {
        app_802494D8();
        a0.i[IX] = mq_app_vi;
        a1.i[IX] = 0;
        a2.i[IX] = OS_MESG_BLOCK;
        lib_osRecvMesg();
        a0.i[IX] = mq_app_vi;
        a1.i[IX] = 0;
        a2.i[IX] = OS_MESG_BLOCK;
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
/* see osSpTaskStartGo */
LIB_SV(osSpTaskYield)
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

#ifdef APP_UNK4
#include "lib/osSendMesg.c"
#include "lib/osStopThread.c"
#include "lib/osRecvMesg.c"
LIB_S0(osSetIntMask)
#include "lib/sinf.c"
LIB_SV(osSpTaskLoad)
#include "lib/osSpTaskStartGo.c"
#include "lib/osDestroyThread.c"

void lib___ull_div(void)
{
    puts("__ull_div");
    exit(EXIT_FAILURE);
}

void lib___ll_mul(void)
{
    puts("__ll_mul");
    exit(EXIT_FAILURE);
}

#include "lib/bzero.c"

LIB_S0(__osMotorAccess)
LIB_S0(osMotorInit)
LIB_S0(osContReset)

void lib_osEepromWrite(void)
{
    puts("osEepromWrite");
    exit(EXIT_FAILURE);
}

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

/* see osSpTaskStartGo */
void lib_osSpTaskYield(void)
{
    puts("osSpTaskYield");
    exit(EXIT_FAILURE);
}

void lib_80030794(void)
{
    puts("80030794");
    exit(EXIT_FAILURE);
}

#include "lib/guMtxIdentF.c"
LIB_SV(osViSetMode)

void lib_osPfsAllocateFile(void)
{
    puts("osPfsAllocateFile");
    exit(EXIT_FAILURE);
}

LIB_S0(osGetCount)

void lib_osEepromProbe(void)
{
    puts("osEepromProbe");
    exit(EXIT_FAILURE);
}

void lib_osPfsFindFile(void)
{
    puts("osPfsFindFile");
    exit(EXIT_FAILURE);
}

LIB_SV(osCreatePiManager)
#include "lib/osSetEventMesg.c"
#include "lib/sqrtf.c"

void lib_osAfterPreNMI(void)
{
    puts("osAfterPreNMI");
    exit(EXIT_FAILURE);
}

#include "lib/osContStartQuery.c"
#include "lib/osContGetQuery.c"

void lib__Printf(void)
{
    puts("_Printf");
    exit(EXIT_FAILURE);
}

#include "lib/osEPiStartDma.c"
#include "lib/memcpy.c"
#include "lib/osCreateMesgQueue.c"
LIB_SV(osInvalICache)
LIB_SV(osInvalDCache)
#include "lib/osEepromLongWrite.c"
#include "lib/osSetThreadPri.c"

void lib_osGetThreadPri(void)
{
    puts("osGetThreadPri");
    exit(EXIT_FAILURE);
}

LIB_SV(osViSwapBuffer)
#include "lib/guMtxXFMF.c"
#include "lib/guMtxCatF.c"

void lib_osSpTaskYielded(void)
{
    puts("osSpTaskYielded");
    exit(EXIT_FAILURE);
}

#include "lib/osGetTime.c"
#include "lib/osAiSetFrequency.c"
#include "lib/guNormalize.c"

void lib___osGetActiveQueue(void)
{
    puts("__osGetActiveQueue");
    exit(EXIT_FAILURE);
}

#include "lib/alCopy.c"

void lib_osPfsDeleteFile(void)
{
    puts("osPfsDeleteFile");
    exit(EXIT_FAILURE);
}

#include "lib/cosf.c"

void lib_osSetTime(void)
{
    puts("osSetTime");
    exit(EXIT_FAILURE);
}

#include "lib/osViSetEvent.c"

LIB_S0(osCartRomInit)
LIB_SV(guS2DInitBg)

void lib_80035D30(void)
{
    puts("80035D30");
    exit(EXIT_FAILURE);
}

#include "lib/alCents2Ratio.c"

void lib_osDpSetNextBuffer(void)
{
    puts("osDpSetNextBuffer");
    exit(EXIT_FAILURE);
}

LIB_SV(osCreateViManager)
LIB_SV(osWritebackDCacheAll)
#include "lib/osStartThread.c"
LIB_SV(osViSetYScale)
#include "lib/osAiSetNextBuffer.c"

void lib_osEepromRead(void)
{
    puts("osEepromRead");
    exit(EXIT_FAILURE);
}

LIB_S0(osViGetCurrentFramebuffer)
/* ext */
#include "lib/osAiGetLength.c"
#endif
