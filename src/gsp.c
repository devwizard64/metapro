#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef GEKKO
#include <malloc.h>
#endif

#include <math.h>

#ifndef GEKKO
#include <GL/gl.h>

#ifdef __3DS__
#include <GL/picaGL.h>
#define glVertex3s(x, y, z) glVertex3f(x, y, z)
#define GSP_SWFOG
#endif

#ifndef GL_CLAMP_TO_EDGE
#define GL_CLAMP_TO_EDGE 0x812F
#endif
#ifndef GL_MIRRORED_REPEAT
#define GL_MIRRORED_REPEAT 0x8370
#endif

extern void glFogCoordf(GLfloat coord);
#endif

#include "types.h"
#include "app.h"
#include "cpu.h"
#include "lib.h"

#ifndef APP_SEQ

#include "gbi.h"

#define G_OML_CYCLE (G_AC_THRESHOLD | G_RM_OPA_SURF | G_RM_OPA_SURF2)

#ifdef GSP_F3DEX
#define GSP_VTX_LEN     0x20
#define GSP_TRI_LEN     2
#else
#define GSP_VTX_LEN     0x10
#define GSP_TRI_LEN     10
#endif

#define RGBA16_R(x)     (((x) >> 8 & 0xF8) | ((x) >> 13       ))
#define RGBA16_G(x)     (((x) >> 3 & 0xF8) | ((x) >>  8 & 0x07))
#define RGBA16_B(x)     (((x) << 2 & 0xF8) | ((x) >>  3 & 0x07))
#define RGBA16_A(x)     (((x) & 0x0001) ? 0xFF : 0x00)
#define IA4_IH(x)       (((x) >> 5       ) * 0x49/2)
#define IA4_IL(x)       (((x) >> 1 & 0x07) * 0x49/2)
#define IA4_AH(x)       (((x) & 0x10) ? 0xFF : 0x00)
#define IA4_AL(x)       (((x) & 0x01) ? 0xFF : 0x00)
#define IA4_IAH(x) \
    (((x) << 0 & 0xE0) | ((x) >> 3 & 0x10) | (((x) & 0x10) ? 0x0F : 0x00))
#define IA4_IAL(x) \
    (((x) << 4 & 0xE0) | ((x) << 1 & 0x10) | (((x) & 0x01) ? 0x0F : 0x00))
#define IA4_AIH(x) \
    (((x) >> 4 & 0x0E) | ((x) >> 7 & 0x01) | (((x) & 0x10) ? 0xF0 : 0x00))
#define IA4_AIL(x) \
    (((x) >> 0 & 0x0E) | ((x) >> 3 & 0x01) | (((x) & 0x01) ? 0xF0 : 0x00))
#define IA8_I(x)        (((x) << 0 & 0xF0) | ((x) >> 4       ))
#define IA8_A(x)        (((x) << 4       ) | ((x) >> 0 & 0x0F))
#define I4_IH           IA8_I
#define I4_IL           IA8_A

#define MP gsp_mtxf_projection
#define MM (*gsp_mtxf_modelview)

#ifdef GEKKO
#define GDP_TF_POINT    GX_NEAR
#define GDP_TF_BILERP   GX_LINEAR
#define GDP_TX_NULL     GX_PASSCLR
#define GDP_TX_MODULATE GX_MODULATE
#define GDP_TX_DECAL    GX_DECAL
#define GDP_TX_BLEND    GX_BLEND
typedef u8 GDP_TF;
typedef u8 GDP_FMT;
typedef u8 GDP_TX;
#else
#define GDP_TF_POINT    GL_NEAREST
#define GDP_TF_BILERP   GL_LINEAR
#define GDP_TX_NULL     -1
#define GDP_TX_MODULATE GL_MODULATE
#define GDP_TX_DECAL    GL_DECAL
#define GDP_TX_BLEND    GL_BLEND
typedef GLuint GDP_TF;
typedef GLint  GDP_FMT;
typedef GLint  GDP_TX;
#endif

struct tile
{
    u16 line;
    u16 tmem;
    u8  pal;
    u8  fmt;
    u8  cm[2];
    u8  shift[2];
    u16 ul[2];
    u16 lr[2];
};

struct txtcache
{
    struct txtcache *next;
    struct tile tile;
    u8 *timg;
    GDP_TF tf;
#ifdef GEKKO
    u8 *buf;
    GXTexObj obj;
#else
    GLuint name;
#endif
};

struct vp
{
#ifdef __EB__
    s16 w;
    s16 h;
    s16 d;
    s16 g;
    s16 x;
    s16 y;
    s16 z;
    s16 f;
#else
    s16 h;
    s16 w;
    s16 g;
    s16 d;
    s16 y;
    s16 x;
    s16 f;
    s16 z;
#endif
};

struct vtx
{
#ifdef __EB__
    s16 x;
    s16 y;
    s16 z;
    u16 f;
    s16 s;
    s16 t;
    s8  r;
    s8  g;
    s8  b;
    u8  a;
#else
    s16 y;
    s16 x;
    u16 f;
    s16 z;
    s16 t;
    s16 s;
    u8  a;
    s8  b;
    s8  g;
    s8  r;
#endif
};

struct vtxf
{
    f32 s;
    f32 t;
    u8  shade[4];
};

struct light
{
    struct
    {
    #ifdef __EB__
        u8 r;
        u8 g;
        u8 b;
        u8 f;
    #else
        u8 f;
        u8 b;
        u8 g;
        u8 r;
    #endif
    }
    col[2];
#ifdef __EB__
    s8 x;
    s8 y;
    s8 z;
    u8 f;
#else
    u8 f;
    s8 z;
    s8 y;
    s8 x;
#endif
#ifdef GSP_F3D
    u32 pad[1];
#endif
#ifdef GSP_F3DEX2
    u32 pad[3];
#endif
};

struct lightf
{
    f32 r;
    f32 g;
    f32 b;
    f32 x;
    f32 y;
    f32 z;
};

struct obj_bg
{
    u16 image_w;
    u16 image_x;
    u16 frame_w;
    s16 frame_x;
    u16 image_h;
    u16 image_y;
    u16 frame_h;
    s16 frame_y;
    u32 image_ptr;
    u8  image_siz;
    u8  image_fmt;
    u16 image_load;
    u16 image_flip;
    u16 image_pal;
    u16 scale_h;
    u16 scale_w;
    s32 image_y_orig;
    u32 pad;
};

struct obj_sprite
{
    u16 scale_w;
    s16 obj_x;
    u16 pad_x;
    u16 image_w;
    u16 scale_h;
    s16 obj_y;
    u16 pad_y;
    u16 image_h;
    u16 image_adrs;
    u16 image_stride;
    u8  image_flag;
    u8  image_pal;
    u8  image_siz;
    u8  image_fmt;
};

struct obj_mtx
{
    s32 a;
    s32 b;
    s32 c;
    s32 d;
    s16 y;
    s16 x;
    u16 base_scale_y;
    u16 base_scale_x;
};

struct obj_sub_mtx
{
    s16 y;
    s16 x;
    u16 base_scale_y;
    u16 base_scale_x;
};

struct obj_txtr
{
    u32 type;
    u32 image;
    u16 twidth; /* tsize for block, pnum for tlut */
    u16 tmem; /* phead for tlut */
    u16 sid;
    u16 theight; /* tline for block */
    u32 flag;
    u32 mask;
};

typedef void GSP(u32 w0, u32 w1);
typedef void *GDP_TEXTURE(GDP_FMT *fmt, const u8 *src, uint w, uint h);
typedef void GDP_TRIANGLE(const u8 *t);
typedef void GDP_COMBINE(u8 *col, struct vtxf *vf);

static GDP_TRIANGLE *gdp_triangle;
static GDP_COMBINE *gdp_combine_cc;
static GDP_COMBINE *gdp_combine_ac;
static struct txtcache *gdp_txtcache;
static GDP_TF gdp_tf;

static u8 gdp_tmem[0x1000];
static struct tile gdp_tile[8];
static u8 *gdp_timg;
static u16 gdp_timg_w;

static u8  gdp_env[4];
static u8  gdp_prim[4];
#if 0
static u8  gdp_blend[4];
#endif
static u8  gdp_fog[4];
static u16 gdp_fill;

static u32 gdp_texrect[4];

static f32 gdp_texture_scale[2];

static u32 gdp_combine_w0;
static u32 gdp_combine_w1;
static u32 gdp_othermode_h;
static u32 gdp_othermode_l;
static u32 gdp_cycle;
static u8  gdp_rect;

static struct vp     gsp_viewport;
static struct light  gsp_light_buf[10];
static struct lightf gsp_lightf_buf[10];
static struct vtx    gsp_vtx_buf[GSP_VTX_LEN];
static struct vtxf   gsp_vtxf_buf[GSP_VTX_LEN];
#ifdef APP_UNK4
static s16  gsp_mtx[32];
#endif
static f32  gsp_mtxf_projection[4][4];
static f32  gsp_mtxf_modelview_stack[16][4][4];
#ifndef GEKKO
static f32  gsp_mtxf_mvp[4][4];
#endif
static f32  MM[4][4];
static u8  *gsp_addr_table[16];
static u32 *gsp_dl_stack[10];
static s8   gsp_dl_index;
static u16  gsp_texture_scale[2];
static u32  gsp_geometry_mode;
static s16  gsp_fog_m;
static s16  gsp_fog_o;
static u8   gsp_light_no;
static bool gsp_lookat;
static bool gsp_new_cache;
static bool gsp_new_texture;
static bool gsp_new_fog;
static bool gsp_new_light;

#ifdef GSP_F3DEX2
static u8 gsp_obj_rendermode;
#endif

#ifdef __3DS__
static f32  gsp_depth;
#endif

static const u8 gdp_rect_tri[][3] = {{0, 1, 2}, {0, 2, 3}};

#if 0
static const char *const str_im_fmt[] = {"RGBA", "YUV", "CI", "IA", "I"};
#endif

static void *gsp_addr(PTR addr)
{
    return &gsp_addr_table[addr >> 24 & 0x0F][addr >>  0 & 0x00FFFFFF];
}

#define i (w*(ty+iy)+(tx+ix))

static void *gdp_texture_rgba16(GDP_FMT *fmt, const u8 *src, uint w, uint h)
{
#ifdef GEKKO
    void *buf;
    u8   *dst;
    uint ty;
    uint tx;
    uint iy;
    uint ix;
    *fmt = GX_TF_RGBA8;
    buf = dst = memalign(0x20, 4*w*h);
    for (ty = 0; ty < h; ty += 4)
    {
        for (tx = 0; tx < w; tx += 4)
        {
            for (iy = 0; iy < 4; iy++)
            {
                for (ix = 0; ix < 4; ix++)
                {
                    uint x = src[2*i+0] << 8 | src[2*i+1];
                    dst[0x00] = RGBA16_A(x);
                    dst[0x01] = RGBA16_R(x);
                    dst[0x20] = RGBA16_G(x);
                    dst[0x21] = RGBA16_B(x);
                    dst += 2;
                }
            }
            dst += 0x20;
        }
    }
    DCFlushRange(buf, 4*w*h);
    return buf;
#else
    void *buf;
    u8   *dst;
    uint  len;
#ifdef __NATIVE__
    *fmt = GL_RGBA8;
    buf = dst = malloc(4*w*h);
#endif
#ifdef __3DS__
    *fmt = GPU_RGBA5551;
    buf = dst = malloc(2*w*h);
#endif
    len = w*h;
    do
    {
    #ifdef __NATIVE__
        uint x = src[0] << 8 | src[1];
        dst[0] = RGBA16_R(x);
        dst[1] = RGBA16_G(x);
        dst[2] = RGBA16_B(x);
        dst[3] = RGBA16_A(x);
        dst += 4;
    #endif
    #ifdef __3DS__
        dst[0] = src[1];
        dst[1] = src[0];
        dst += 2;
    #endif
        src += 2;
        len -= 1;
    }
    while (len > 0);
    return buf;
#endif
}

#ifndef APP_UNSM
static void *gdp_texture_rgba32(GDP_FMT *fmt, const u8 *src, uint w, uint h)
{
#ifdef GEKKO
    void *buf;
    u8   *dst;
    uint ty;
    uint tx;
    uint iy;
    uint ix;
    *fmt = GX_TF_RGBA8;
    buf = dst = memalign(0x20, 4*w*h);
    for (ty = 0; ty < h; ty += 4)
    {
        for (tx = 0; tx < w; tx += 4)
        {
            for (iy = 0; iy < 4; iy++)
            {
                for (ix = 0; ix < 4; ix++)
                {
                    dst[0x00] = src[4*i+3];
                    dst[0x01] = src[4*i+2];
                    dst[0x20] = src[4*i+1];
                    dst[0x21] = src[4*i+0];
                    dst += 2;
                }
            }
            dst += 0x20;
        }
    }
    DCFlushRange(buf, 4*w*h);
    return buf;
#else
#ifdef __NATIVE__
    *fmt = GL_RGBA8;
#endif
#ifdef __3DS__
    *fmt = GPU_RGBA8;
#endif
    return memcpy(malloc(4*w*h), src, 4*w*h);
#endif
}
#else
#define gdp_texture_rgba32 NULL
#endif

#define gdp_texture_yuv16  NULL

static void *gdp_texture_ia4(GDP_FMT *fmt, const u8 *src, uint w, uint h)
{
#ifdef GEKKO
    void *buf;
    u8   *dst;
    uint ty;
    uint tx;
    uint iy;
    uint ix;
    *fmt = GX_TF_IA4;
    buf = dst = memalign(0x20, w*h);
    for (ty = 0; ty < h; ty += 4)
    {
        for (tx = 0; tx < w; tx += 8)
        {
            for (iy = 0; iy < 4; iy++)
            {
                for (ix = 0; ix < 8; ix += 2)
                {
                    dst[0] = IA4_AIH(src[i/2]);
                    dst[1] = IA4_AIL(src[i/2]);
                    dst += 2;
                }
            }
        }
    }
    DCFlushRange(buf, w*h);
    return buf;
#else
    void *buf;
    u8   *dst;
    uint  len;
#ifdef __NATIVE__
    *fmt = GL_RGBA8;
    buf = dst = malloc(4*w*h);
#endif
#ifdef __3DS__
    *fmt = GPU_LA4;
    buf = dst = malloc(w*h);
#endif
    len = w*h/2;
    do
    {
    #ifdef __NATIVE__
        dst[0] = dst[1] = dst[2] = IA4_IH(src[0]);
        dst[3]                   = IA4_AH(src[0]);
        dst[4] = dst[5] = dst[6] = IA4_IL(src[0]);
        dst[7]                   = IA4_AL(src[0]);
        dst += 8;
    #endif
    #ifdef __3DS__
        dst[0] = IA4_IAH(src[0]);
        dst[1] = IA4_IAL(src[0]);
        dst += 2;
    #endif
        src += 1;
        len -= 1;
    }
    while (len > 0);
    return buf;
#endif
}

static void *gdp_texture_ia8(GDP_FMT *fmt, const u8 *src, uint w, uint h)
{
#ifdef GEKKO
    void *buf;
    u8   *dst;
    uint ty;
    uint tx;
    uint iy;
    uint ix;
    *fmt = GX_TF_IA4;
    buf = dst = memalign(0x20, w*h);
    for (ty = 0; ty < h; ty += 4)
    {
        for (tx = 0; tx < w; tx += 8)
        {
            for (iy = 0; iy < 4; iy++)
            {
                for (ix = 0; ix < 8; ix++)
                {
                    dst[0] = src[i] << 4 | src[i] >> 4;
                    dst += 1;
                }
            }
        }
    }
    DCFlushRange(buf, w*h);
    return buf;
#else
#ifdef __3DS__
    *fmt = GPU_LA4;
    return memcpy(malloc(w*h), src, w*h);
#else
    void *buf;
    u8   *dst;
    uint  len;
#ifdef __NATIVE__
    *fmt = GL_RGBA8;
    buf = dst = malloc(4*w*h);
#endif
    len = w*h;
    do
    {
    #ifdef __NATIVE__
        dst[0] = dst[1] = dst[2] = IA8_I(src[0]);
        dst[3]                   = IA8_A(src[0]);
        dst += 4;
    #endif
        src += 1;
        len -= 1;
    }
    while (len > 0);
    return buf;
#endif
#endif
}

#ifdef APP_UNSM
static void *gdp_texture_ia8_face(GDP_FMT *fmt, const u8 *src, uint w, uint h)
{
#ifdef GEKKO
    void *buf;
    u8   *dst;
    uint ty;
    uint tx;
    uint iy;
    uint ix;
    *fmt = GX_TF_IA4;
    buf = dst = memalign(0x20, w*h);
    for (ty = 0; ty < h; ty += 4)
    {
        for (tx = 0; tx < w; tx += 8)
        {
            for (iy = 0; iy < 4; iy++)
            {
                for (ix = 0; ix < 8; ix++)
                {
                    dst[0] = 0x0F | src[i];
                    dst += 1;
                }
            }
        }
    }
    DCFlushRange(buf, w*h);
    return buf;
#else
    void *buf;
    u8   *dst;
    uint  len;
#ifdef __NATIVE__
    *fmt = GL_RGBA8;
    buf = dst = malloc(4*w*h);
#endif
#ifdef __3DS__
    *fmt = GPU_LA4;
    buf = dst = malloc(w*h);
#endif
    len = w*h;
    do
    {
    #ifdef __NATIVE__
        dst[0] = dst[1] = dst[2] = 0xFF;
        dst[3]                   = IA8_I(src[0]);
        dst += 4;
    #endif
    #ifdef __3DS__
        dst[0] = 0xF0 | (src[0] >> 4);
        dst += 1;
    #endif
        src += 1;
        len -= 1;
    }
    while (len > 0);
    return buf;
#endif
}
#endif

static void *gdp_texture_ia16(GDP_FMT *fmt, const u8 *src, uint w, uint h)
{
#ifdef GEKKO
    void *buf;
    u8   *dst;
    uint ty;
    uint tx;
    uint iy;
    uint ix;
    *fmt = GX_TF_IA8;
    buf = dst = memalign(0x20, 2*w*h);
    for (ty = 0; ty < h; ty += 4)
    {
        for (tx = 0; tx < w; tx += 4)
        {
            for (iy = 0; iy < 4; iy++)
            {
                for (ix = 0; ix < 4; ix++)
                {
                    dst[0] = src[2*i+1];
                    dst[1] = src[2*i+0];
                    dst += 2;
                }
            }
        }
    }
    DCFlushRange(buf, 2*w*h);
    return buf;
#else
    void *buf;
    u8   *dst;
    uint  len;
#ifdef __NATIVE__
    *fmt = GL_RGBA8;
    buf = dst = malloc(4*w*h);
#endif
#ifdef __3DS__
    *fmt = GPU_LA8;
    buf = dst = malloc(2*w*h);
#endif
    len = w*h;
    do
    {
    #ifdef __NATIVE__
        dst[0] = dst[1] = dst[2] = src[0];
        dst[3]                   = src[1];
        dst += 4;
    #endif
    #ifdef __3DS__
        dst[0] = src[1];
        dst[1] = src[0];
        dst += 2;
    #endif
        src += 2;
        len -= 1;
    }
    while (len > 0);
    return buf;
#endif
}

#ifndef APP_UNSM
static void *gdp_texture_i4(GDP_FMT *fmt, const u8 *src, uint w, uint h)
{
#ifdef GEKKO
    void *buf;
    u8   *dst;
    uint ty;
    uint tx;
    uint iy;
    uint ix;
    *fmt = GX_TF_IA4;
    buf = dst = memalign(0x20, w*h);
    for (ty = 0; ty < h; ty += 4)
    {
        for (tx = 0; tx < w; tx += 8)
        {
            for (iy = 0; iy < 4; iy++)
            {
                for (ix = 0; ix < 8; ix += 2)
                {
                    dst[0] = I4_IH(src[i/2]);
                    dst[1] = I4_IL(src[i/2]);
                    dst += 2;
                }
            }
        }
    }
    DCFlushRange(buf, w*h);
    return buf;
#else
    void *buf;
    u8   *dst;
    uint  len;
#ifdef __NATIVE__
    *fmt = GL_RGBA8;
    buf = dst = malloc(4*w*h);
#endif
#ifdef __3DS__
    *fmt = GPU_LA4;
    buf = dst = malloc(w*h);
#endif
    len = w*h/2;
    do
    {
    #ifdef __NATIVE__
        dst[0] = dst[1] = dst[2] = dst[3] = I4_IH(src[0]);
        dst[4] = dst[5] = dst[6] = dst[7] = I4_IL(src[0]);
        dst += 8;
    #endif
    #ifdef __3DS__
        dst[0] = I4_IH(src[0]);
        dst[1] = I4_IL(src[0]);
        dst += 2;
    #endif
        src += 1;
        len -= 1;
    }
    while (len > 0);
    return buf;
#endif
}

static void *gdp_texture_i8(GDP_FMT *fmt, const u8 *src, uint w, uint h)
{
#ifdef GEKKO
    void *buf;
    u8   *dst;
    uint ty;
    uint tx;
    uint iy;
    uint ix;
    *fmt = GX_TF_IA8;
    buf = dst = memalign(0x20, 2*w*h);
    for (ty = 0; ty < h; ty += 4)
    {
        for (tx = 0; tx < w; tx += 4)
        {
            for (iy = 0; iy < 4; iy++)
            {
                for (ix = 0; ix < 4; ix++)
                {
                    dst[0] = dst[1] = src[i];
                    dst += 2;
                }
            }
        }
    }
    DCFlushRange(buf, 2*w*h);
    return buf;
#else
    void *buf;
    u8   *dst;
    uint  len;
#ifdef __NATIVE__
    *fmt = GL_RGBA8;
    buf = dst = malloc(4*w*h);
#endif
#ifdef __3DS__
    *fmt = GPU_LA8;
    buf = dst = malloc(2*w*h);
#endif
    len = w*h;
    do
    {
    #ifdef __NATIVE__
        dst[0] = dst[1] = dst[2] = dst[3] = src[0];
        dst += 4;
    #endif
    #ifdef __3DS__
        dst[0] = dst[1] = src[0];
        dst += 2;
    #endif
        src += 1;
        len -= 1;
    }
    while (len > 0);
    return buf;
#endif
}
#else
#define gdp_texture_i4     NULL
#define gdp_texture_i8     NULL
#endif

#undef i

#ifdef APP_UNSM
#define gdp_texture_ci4    NULL
#define gdp_texture_ci8    NULL
#else
static void *gdp_texture_ci(GDP_FMT *fmt, u8 *src, uint w, uint h)
{
    void *dst;
    if ((gdp_othermode_h & (1 << G_MDSFT_TEXTLUT)) == 0)
    {
        dst = gdp_texture_rgba16(fmt, src, w, h);
    }
    else
    {
        dst = gdp_texture_ia16(fmt, src, w, h);
    }
    free(src);
    return dst;
}

static void *gdp_texture_ci4(
    GDP_FMT *fmt, const u8 *src, uint w, uint h
)
{
    void *buf = malloc(2*w*h);
    u16 *tlut = (u16 *)&gdp_tmem[0x800 + 0x80*gdp_tile[0].pal];
    u16 *dst = buf;
    uint len = w*h;
    do
    {
        dst[0] = tlut[src[0] >> 4       ];
        dst[1] = tlut[src[0] >> 0 & 0x0F];
        dst += 2;
        src += 1;
        len -= 1;
    }
    while (len > 0);
    return gdp_texture_ci(fmt, buf, w, h);
}

static void *gdp_texture_ci8(
    GDP_FMT *fmt, const u8 *src, uint w, uint h
)
{
    void *buf = malloc(2*w*h);
    u16 *tlut = (u16 *)&gdp_tmem[0x800 + 0x80*gdp_tile[0].pal];
    u16 *dst = buf;
    uint len = w*h;
    do
    {
        dst[0] = tlut[src[0]];
        dst += 1;
        src += 1;
        len -= 1;
    }
    while (len > 0);
    return gdp_texture_ci(fmt, buf, w, h);
}
#endif

static GDP_TEXTURE *gdp_texture_table[] =
{
    /* RGBA  4 */ NULL,
    /* RGBA  8 */ NULL,
    /* RGBA 16 */ gdp_texture_rgba16,
    /* RGBA 32 */ gdp_texture_rgba32,
    /* YUV   4 */ NULL,
    /* YUV   8 */ NULL,
    /* YUV  16 */ gdp_texture_yuv16,
    /* YUV  32 */ NULL,
    /* CI    4 */ gdp_texture_ci4,
    /* CI    8 */ gdp_texture_ci8,
    /* CI   16 */ NULL,
    /* CI   32 */ NULL,
    /* IA    4 */ gdp_texture_ia4,
    /* IA    8 */ gdp_texture_ia8,
    /* IA   16 */ gdp_texture_ia16,
    /* IA   32 */ NULL,
    /* I     4 */ gdp_texture_i4,
    /* I     8 */ gdp_texture_i8,
    /* I    16 */ NULL,
    /* I    32 */ NULL,
};

#ifdef GEKKO
static const u8 gdp_texture_cm_table[] =
{
    /* 0x00 G_TX_NOMIRROR | G_TX_WRAP  */ GX_REPEAT,
    /* 0x01 G_TX_MIRROR   | G_TX_WRAP  */ GX_MIRROR,
    /* 0x02 G_TX_NOMIRROR | G_TX_CLAMP */ GX_CLAMP,
    /* 0x03 G_TX_MIRROR   | G_TX_CLAMP */ GX_CLAMP,
};
#else
static const GLuint gdp_texture_cm_table[] =
{
    /* 0x00 G_TX_NOMIRROR | G_TX_WRAP  */ GL_REPEAT,
    /* 0x01 G_TX_MIRROR   | G_TX_WRAP  */ GL_MIRRORED_REPEAT,
    /* 0x02 G_TX_NOMIRROR | G_TX_CLAMP */ GL_CLAMP_TO_EDGE,
    /* 0x03 G_TX_MIRROR   | G_TX_CLAMP */ GL_CLAMP_TO_EDGE,
};
#endif

static void gsp_flush_texture(void)
{
#define cms gdp_texture_cm_table[tile->cm[0]]
#define cmt gdp_texture_cm_table[tile->cm[1]]
    struct tile *tile = &gdp_tile[0];
    uint w = tile->lr[0] - tile->ul[0];
    uint h = tile->lr[1] - tile->ul[1];
    struct txtcache *txtcache;
    GDP_TEXTURE *texture;
    gdp_texture_scale[0] = (1.0F/32) / w;
    gdp_texture_scale[1] = (1.0F/32) / h;
    txtcache = gdp_txtcache;
    while (txtcache != NULL)
    {
        if
        (
            txtcache->timg == gdp_timg &&
            txtcache->tf   == gdp_tf   &&
            memcmp(&txtcache->tile, tile, sizeof(struct tile)) == 0
        )
        {
        #ifdef GEKKO
            GX_InvalidateTexAll();
            GX_LoadTexObj(&txtcache->obj, GX_TEXMAP0);
        #else
            glBindTexture(GL_TEXTURE_2D, txtcache->name);
        #endif
            return;
        }
        txtcache = txtcache->next;
    }
    texture = gdp_texture_table[tile->fmt];
    if (texture != NULL)
    {
        u8 *src = &gdp_tmem[tile->tmem];
    #ifndef GEKKO
        u8 *buf;
    #endif
        GDP_FMT fmt;
        txtcache = malloc(sizeof(struct txtcache));
        txtcache->next = gdp_txtcache;
        gdp_txtcache = txtcache;
        memcpy(&txtcache->tile, tile, sizeof(struct tile));
        txtcache->timg = gdp_timg;
        txtcache->tf = gdp_tf;
    #ifdef GEKKO
        txtcache->buf = texture(&fmt, src, w, h);
        GX_InitTexObj(
            &txtcache->obj, txtcache->buf, w, h, fmt, cms, cmt, GX_FALSE
        );
        GX_InitTexObjFilterMode(&txtcache->obj, gdp_tf, gdp_tf);
        GX_InvalidateTexAll();
        GX_LoadTexObj(&txtcache->obj, GX_TEXMAP0);
    #else
        buf = texture(&fmt, src, w, h);
        glFlush();
        glGenTextures(1, &txtcache->name);
        glBindTexture(GL_TEXTURE_2D, txtcache->name);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gdp_tf);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gdp_tf);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, cms);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, cmt);
        glTexImage2D(
            GL_TEXTURE_2D, 0, fmt, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, buf
        );
        free(buf);
    #endif
    }
    else
    {
    #if 0
        wdebug(
            "unknown texture fmt G_IM_FMT_%s, G_IM_SIZ_%db\n",
            str_im_fmt[tile->fmt >> 2], 4 << (tile->fmt & 3)
        );
    #endif
    }
#undef cms
#undef cmt
}

static void gsp_flush_fog(void)
{
#ifndef GSP_SWFOG
#ifdef GEKKO
    GXColor col;
    col.r = gdp_fog[0];
    col.g = gdp_fog[1];
    col.b = gdp_fog[2];
    col.a = gdp_fog[3];
#endif
    if (gsp_geometry_mode & G_FOG)
    {
    #ifdef GEKKO
        float start = 1 - (float)gsp_fog_o/gsp_fog_m;
        float end   = start + 256.0F/gsp_fog_m;
        GX_SetFog(GX_FOG_LIN, start, end, 1, 2, col);
    #else
        GLfloat col[4];
        col[0] = (1.0F/0xFF) * gdp_fog[0];
        col[1] = (1.0F/0xFF) * gdp_fog[1];
        col[2] = (1.0F/0xFF) * gdp_fog[2];
        col[3] = (1.0F/0xFF) * gdp_fog[3];
        glEnable(GL_FOG);
        glFogfv(GL_FOG_COLOR, col);
    #endif
    }
    else
    {
    #ifdef GEKKO
        GX_SetFog(GX_FOG_NONE, 0, 0, 0, 0, col);
    #else
        glDisable(GL_FOG);
    #endif
    }
#endif
}

static void gdp_tri(const u8 *t)
{
    uint i;
    if (gsp_new_texture)    {gsp_new_texture = false; gsp_flush_texture();}
    if (gsp_new_fog)        {gsp_new_fog     = false; gsp_flush_fog();    }
#ifdef GEKKO
    GX_Begin(GX_TRIANGLES, GX_VTXFMT0, 3);
#else
    glBegin(GL_TRIANGLES);
#endif
    for (i = 0; i < 3; i++)
    {
        struct vtx  *v  = &gsp_vtx_buf[t[i]];
        struct vtxf *vf = &gsp_vtxf_buf[t[i]];
        float s = gdp_texture_scale[0] * vf->s;
        float t = gdp_texture_scale[1] * vf->t;
        u8 col[4];
        gdp_combine_cc(col, vf);
        gdp_combine_ac(col, vf);
    #ifdef GEKKO
        GX_Position3s16(v->x, v->y, v->z);
        GX_Color4u8(col[0], col[1], col[2], col[3]);
        GX_TexCoord2f32(s, t);
    #else
        glColor4ub(col[0], col[1], col[2], col[3]);
        glTexCoord2f(s, t);
    #ifndef GSP_SWFOG
        glFogCoordf(vf->shade[3]);
    #endif
        glVertex3s(v->x, v->y, v->z);
    #endif
    }
#ifdef GEKKO
    GX_End();
#else
    glEnd();
#endif
}

#ifdef GSP_SWFOG
static void gdp_tri_fog(const u8 *t)
{
    gdp_tri(t);
#define sa(i) gsp_vtxf_buf[t[i]].shade[3]
    if (sa(0) > 0x00 || sa(1) > 0x00 || sa(2) > 0x00)
#undef sa
    {
        gdp_set_tx(GDP_TX_NULL);
        gdp_combine_cc = gdp_combine_cc_fog;
        gdp_combine_ac = gdp_combine_ac_fog;
        gdp_set_rm(G_RM_AA_ZB_XLU_DECAL | G_RM_AA_ZB_XLU_DECAL2);
        gdp_tri(t);
        gdp_flush_cc();
        gdp_flush_rm();
    }
}
#endif

#if 0
static void gdp_tri_special(const u8 *t)
{
    gdp_tri(t);
    gdp_set_rm(G_RM_AA_ZB_XLU_DECAL | G_RM_AA_ZB_XLU_DECAL2);
    gdp_tri(t);
    gdp_flush_rm();
}
#endif

#define gsp_flush_mp()          gsp_set_mp(MP)
#define gsp_flush_mm()          gsp_set_mm(MM)
#define gsp_flush_vp()          \
    gsp_set_vp(                                 \
        gsp_viewport.x - gsp_viewport.w,        \
        gsp_viewport.x + gsp_viewport.w,        \
        gsp_viewport.y - gsp_viewport.h,        \
        gsp_viewport.y + gsp_viewport.h         \
    )
#define gsp_flush_cull()        gsp_set_cull(gsp_geometry_mode & G_CULL_BOTH)
#define gdp_flush_cc()          gdp_set_cc(gdp_combine_w0, gdp_combine_w1)
#define gdp_flush_rm()          gdp_set_rm(gdp_othermode_l)

#define TRANSPOSE(i)            \
{                               \
    mt[i][0] = mf[0][i];        \
    mt[i][1] = mf[1][i];        \
    mt[i][2] = mf[2][i];        \
    mt[i][3] = mf[3][i];        \
}

#ifdef GEKKO
static bool gsp_decal = false;
#endif
static void gsp_set_mp(f32 mf[4][4])
{
#ifdef GEKKO
    Mtx44 mt;
    TRANSPOSE(0);
    TRANSPOSE(1);
    TRANSPOSE(2);
    TRANSPOSE(3);
    if (gsp_decal) mt[2][3]--;
    GX_LoadProjectionMtx(mt, mt[3][3] != 0 ? GX_ORTHOGRAPHIC : GX_PERSPECTIVE);
#else
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(&mf[0][0]);
#ifdef __3DS__
    glTranslatef(gsp_depth, 0, 0);
#endif
#endif
}

static void gsp_set_mm(f32 mf[4][4])
{
#ifdef GEKKO
    Mtx mt;
    TRANSPOSE(0);
    TRANSPOSE(1);
    TRANSPOSE(2);
    GX_LoadPosMtxImm(mt, GX_PNMTX0);
#else
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(&mf[0][0]);
#endif
}

#undef TRANSPOSE

static void gsp_set_vp(int l, int r, int t, int b)
{
#ifdef GEKKO
    GX_SetViewport(
        (  l) * lib_video_w/1280.0F,
        (  t) * lib_video_h/ 960.0F,
        (r-l) * lib_video_w/1280.0F,
        (b-t) * lib_video_h/ 960.0F,
        0, 1
    );
#else
    glViewport(
        (    l) * lib_video_w/1280,
        (960-b) * lib_video_h/ 960,
        (  r-l) * lib_video_w/1280,
        (  b-t) * lib_video_h/ 960
    );
#endif
}

static void gsp_set_cull(u32 mode)
{
    switch (mode)
    {
    #ifdef GEKKO
        case 0:             GX_SetCullMode(GX_CULL_NONE);   break;
        case G_CULL_FRONT:  GX_SetCullMode(GX_CULL_BACK);   break;
        case G_CULL_BACK:   GX_SetCullMode(GX_CULL_FRONT);  break;
        case G_CULL_BOTH:   GX_SetCullMode(GX_CULL_ALL);    break;
    #else
        case 0:
            glDisable(GL_CULL_FACE);
            break;
        case G_CULL_FRONT:
            glEnable(GL_CULL_FACE);
            glCullFace(GL_FRONT);
            break;
        case G_CULL_BACK:
            glEnable(GL_CULL_FACE);
            glCullFace(GL_BACK);
            break;
        case G_CULL_BOTH:
            glEnable(GL_CULL_FACE);
            glCullFace(GL_FRONT_AND_BACK);
            break;
    #endif
    }
}

static void gdp_set_tx(GDP_TX tx)
{
#ifdef GEKKO
    GX_SetTevOrder(
        GX_TEVSTAGE0,
        tx != GDP_TX_NULL ? GX_TEXCOORD0 : GX_TEXCOORDNULL,
        tx != GDP_TX_NULL ? GX_TEXMAP0   : GX_TEXMAP_NULL,
        GX_COLOR0A0
    );
    GX_SetTevOp(GX_TEVSTAGE0, tx);
#else
    if (tx != GDP_TX_NULL)
    {
        glEnable(GL_TEXTURE_2D);
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, tx);
    }
    else
    {
        glDisable(GL_TEXTURE_2D);
    }
#endif
}

static void gdp_combine_cc_0(u8 *col, unused struct vtxf *vf)
{
    col[0] = 0x00;
    col[1] = 0x00;
    col[2] = 0x00;
}

static void gdp_combine_cc_1(u8 *col, unused struct vtxf *vf)
{
    col[0] = 0xFF;
    col[1] = 0xFF;
    col[2] = 0xFF;
}

static void gdp_combine_cc_shade(u8 *col, struct vtxf *vf)
{
    col[0] = vf->shade[0];
    col[1] = vf->shade[1];
    col[2] = vf->shade[2];
}

static void gdp_combine_cc_prim(u8 *col, unused struct vtxf *vf)
{
    col[0] = gdp_prim[0];
    col[1] = gdp_prim[1];
    col[2] = gdp_prim[2];
}

static void gdp_combine_cc_env(u8 *col, unused struct vtxf *vf)
{
    col[0] = gdp_env[0];
    col[1] = gdp_env[1];
    col[2] = gdp_env[2];
}

static void gdp_combine_cc_shade_env(u8 *col, struct vtxf *vf)
{
    col[0] = vf->shade[0] * gdp_env[0] / 0x100;
    col[1] = vf->shade[1] * gdp_env[1] / 0x100;
    col[2] = vf->shade[2] * gdp_env[2] / 0x100;
}

#ifndef APP_UNSM
static void gdp_combine_cc_shade_prim(u8 *col, struct vtxf *vf)
{
    col[0] = vf->shade[0] * gdp_prim[0] / 0x100;
    col[1] = vf->shade[1] * gdp_prim[1] / 0x100;
    col[2] = vf->shade[2] * gdp_prim[2] / 0x100;
}

static void gdp_combine_cc_prim_env(u8 *col, unused struct vtxf *vf)
{
    col[0] = gdp_prim[0] * gdp_env[0] / 0x100;
    col[1] = gdp_prim[1] * gdp_env[1] / 0x100;
    col[2] = gdp_prim[2] * gdp_env[2] / 0x100;
}

static void gdp_combine_cc_prim_env_shade_env(u8 *col, struct vtxf *vf)
{
    col[0] = (gdp_prim[0]-gdp_env[0]) * vf->shade[0]/0x100 + gdp_env[0];
    col[1] = (gdp_prim[1]-gdp_env[1]) * vf->shade[1]/0x100 + gdp_env[1];
    col[2] = (gdp_prim[2]-gdp_env[2]) * vf->shade[2]/0x100 + gdp_env[2];
}

static void gdp_combine_cc_1env(u8 *col, unused struct vtxf *vf)
{
    col[0] = 0xFF - gdp_env[0];
    col[1] = 0xFF - gdp_env[1];
    col[2] = 0xFF - gdp_env[2];
}
#endif

#ifdef GSP_SWFOG
static void gdp_combine_cc_fog(u8 *col, unused struct vtxf *vf)
{
    col[0] = gdp_fog[0];
    col[1] = gdp_fog[1];
    col[2] = gdp_fog[2];
}
#endif

static void gdp_combine_ac_0(u8 *col, unused struct vtxf *vf)
{
    col[3] = 0x00;
}

static void gdp_combine_ac_1(u8 *col, unused struct vtxf *vf)
{
    col[3] = 0xFF;
}

static void gdp_combine_ac_shade(u8 *col, struct vtxf *vf)
{
    col[3] = vf->shade[3];
}

static void gdp_combine_ac_prim(u8 *col, unused struct vtxf *vf)
{
    col[3] = gdp_prim[3];
}

static void gdp_combine_ac_env(u8 *col, unused struct vtxf *vf)
{
    col[3] = gdp_env[3];
}

static void gdp_combine_ac_shade_env(u8 *col, unused struct vtxf *vf)
{
    col[3] = vf->shade[3] * gdp_env[3] / 0x100;
}

#ifdef GSP_SWFOG
static void gdp_combine_ac_fog(u8 *col, struct vtxf *vf)
{
    col[3] = vf->shade[3];
}
#endif

#define CC(a, b, c, d)                  \
(                                       \
    ((G_CCMUX_##a) << 12 & 0xF000U) |   \
    ((G_CCMUX_##b) <<  8 & 0x0F00U) |   \
    ((G_CCMUX_##c) <<  3 & 0x00F8U) |   \
    ((G_CCMUX_##d) <<  0 & 0x0007U)     \
)
#define AC(a, b, c, d)                  \
(                                       \
    ((G_ACMUX_##a) <<  9 & 0x0E00U) |   \
    ((G_ACMUX_##b) <<  6 & 0x01C0U) |   \
    ((G_ACMUX_##c) <<  3 & 0x0038U) |   \
    ((G_ACMUX_##d) <<  0 & 0x0007U)     \
)
#define CC1(a, b, c, d) ((u32)CC(a, b, c, d) << 16 | (u32)CC(a, b, c, d))
#define AC1(a, b, c, d) ((u32)AC(a, b, c, d) << 12 | (u32)AC(a, b, c, d))
#define CC2(a0, b0, c0, d0, a1, b1, c1, d1) \
    ((u32)CC(a0, b0, c0, d0) << 16 | (u32)CC(a1, b1, c1, d1))
#define AC2(a0, b0, c0, d0, a1, b1, c1, d1) \
    ((u32)AC(a0, b0, c0, d0) << 12 | (u32)AC(a1, b1, c1, d1))
static void gdp_set_cc(u32 w0, u32 w1)
{
    u32 cc;
    u32 ac;
    GDP_TX tx;
    cc =
        (w0 <<  8 & 0xF0000000U) |
        (w1 >>  4 & 0x0F000000U) |
        (w0 <<  4 & 0x00F80000U) |
        (w1 <<  1 & 0x00070000U) |
        (w0 <<  7 & 0x0000F000U) |
        (w1 >> 16 & 0x00000F00U) |
        (w0 <<  3 & 0x000000F8U) |
        (w1 >>  6 & 0x00000007U);
    ac =
        (w0 <<  9 & 0x00E00000U) |
        (w1 <<  6 & 0x001C0000U) |
        (w0 <<  6 & 0x00038000U) |
        (w1 <<  3 & 0x00007000U) |
        (w1 >> 12 & 0x00000E00U) |
        (w1 <<  3 & 0x000001C0U) |
        (w1 >> 15 & 0x00000038U) |
        (w1 <<  0 & 0x00000007U);
    if ((cc & 0x0000FFFFU) == CC(0, 0, 0, COMBINED))
    {
        cc = (cc & 0xFFFF0000U) | cc >> 16;
    }
    if ((ac & 0x00000FFFU) == AC(0, 0, 0, COMBINED))
    {
        ac = (ac & 0x00FFF000U) | ac >> 12;
    }
#ifdef GSP_SWFOG
    gdp_triangle = (gsp_geometry_mode & G_FOG) ? gdp_tri_fog : gdp_tri;
#else
    gdp_triangle = gdp_tri;
#endif
#ifdef APP_UNSM
    gdp_texture_table[G_IM_FMT_IA << 2 | G_IM_SIZ_8b] = gdp_texture_ia8;
#endif
    switch (cc)
    {
    #ifndef APP_UNSM
        case CC1(0, 0, 0, 0):
            gdp_combine_cc = gdp_combine_cc_0;
            tx = GDP_TX_NULL;
            break;
        case CC1(0, 0, 0, 1):
            gdp_combine_cc = gdp_combine_cc_1;
            tx = GDP_TX_NULL;
            break;
    #endif
        case CC1(0, 0, 0, TEXEL0):
        case CC1(TEXEL1, TEXEL0, LOD_FRACTION, TEXEL0):
            gdp_combine_cc = gdp_combine_cc_1;
            tx = GDP_TX_MODULATE;
            break;
        case CC1(0, 0, 0, SHADE):
    #ifndef APP_UNSM
        case CC1(1, 0, SHADE, 0):
    #endif
            gdp_combine_cc = gdp_combine_cc_shade;
            tx = GDP_TX_NULL;
            break;
        case CC1(TEXEL0, 0, SHADE, 0):
            gdp_combine_cc = gdp_combine_cc_shade;
            tx = GDP_TX_MODULATE;
            break;
    #ifndef APP_UNSM
        case CC1(0, 0, 0, PRIMITIVE):
            gdp_combine_cc = gdp_combine_cc_prim;
            tx = GDP_TX_NULL;
            break;
    #endif
        case CC1(TEXEL0, 0, PRIMITIVE, 0):
    #ifndef APP_UNSM
        case CC1(PRIMITIVE, 0, TEXEL0, 0):
    #endif
            gdp_combine_cc = gdp_combine_cc_prim;
            tx = GDP_TX_MODULATE;
            break;
    #ifndef APP_UNSM
        case CC1(0, 0, 0, ENVIRONMENT):
            gdp_combine_cc = gdp_combine_cc_env;
            tx = GDP_TX_NULL;
            break;
    #endif
        case CC1(TEXEL0, 0, ENVIRONMENT, 0):
            gdp_combine_cc = gdp_combine_cc_env;
            tx = GDP_TX_MODULATE;
            break;
        case CC1(SHADE, 0, ENVIRONMENT, 0):
            gdp_combine_cc = gdp_combine_cc_shade_env;
            tx = GDP_TX_NULL;
            break;
    #ifndef APP_UNSM
        case CC1(PRIMITIVE, 0, SHADE, 0):
            gdp_combine_cc = gdp_combine_cc_shade_prim;
            tx = GDP_TX_NULL;
            break;
        case CC2(TEXEL0, 0, SHADE, 0, COMBINED, 0, PRIMITIVE, 0):
            gdp_combine_cc = gdp_combine_cc_shade_prim;
            tx = GDP_TX_MODULATE;
            break;
        case CC1(PRIMITIVE, 0, ENVIRONMENT, 0):
            gdp_combine_cc = gdp_combine_cc_prim_env;
            tx = GDP_TX_NULL;
            break;
        case CC1(PRIMITIVE, ENVIRONMENT, SHADE, ENVIRONMENT):
            gdp_combine_cc = gdp_combine_cc_prim_env_shade_env;
            tx = GDP_TX_NULL;
            break;
        case CC2(
            TEXEL0, 0, PRIMITIVE, 0,
            COMBINED, ENVIRONMENT, SHADE, ENVIRONMENT
        ): /*this is wrong*/
            gdp_combine_cc = gdp_combine_cc_prim_env_shade_env;
            tx = GDP_TX_MODULATE;
            break;
    #endif
        case CC1(TEXEL0, SHADE, TEXEL0_ALPHA, SHADE):
            gdp_combine_cc = gdp_combine_cc_shade;
            tx = GDP_TX_DECAL;
            break;
    #ifdef APP_UNSM
        case CC1(PRIMITIVE, SHADE, TEXEL0, SHADE):
            gdp_combine_cc = gdp_combine_cc_shade;
            tx = GDP_TX_DECAL;
            gdp_texture_table[G_IM_FMT_IA << 2 | G_IM_SIZ_8b] =
                gdp_texture_ia8_face;
            break;
    #endif
    #ifndef APP_UNSM
        case CC1(SHADE, ENVIRONMENT, TEXEL0, ENVIRONMENT): /*this is wrong*/
            gdp_combine_cc = gdp_combine_cc_shade;
            tx = GDP_TX_MODULATE;
            /* C = shade */
            break;
        case CC1(PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT):
            gdp_combine_cc = gdp_combine_cc_prim;
            tx = GDP_TX_MODULATE;
            /* C = prim */
            break;
        case CC2(
            PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT,
            COMBINED, 0, SHADE, 0
        ): /*this is wrong*/
            gdp_combine_cc = gdp_combine_cc_shade_prim;
            tx = GDP_TX_MODULATE;
            /* C = prim */
            break;
        case CC1(1, ENVIRONMENT, TEXEL0, PRIMITIVE): /*this is wrong*/
            gdp_combine_cc = gdp_combine_cc_1env;
            tx = GDP_TX_MODULATE;
            /* C = 1 */
            break;
    #endif
        default:
            gdp_combine_cc = gdp_combine_cc_0;
            tx = GDP_TX_NULL;
            wdebug("unknown cc %08" FMT_X "%08" FMT_X "\n", w0, w1);
            break;
    }
    switch (ac)
    {
    #ifndef APP_UNSM
        case AC1(0, 0, 0, 1):
    #endif
        case AC1(0, 0, 0, TEXEL0):
    #ifndef APP_UNSM
        case AC2(0, 0, 0, COMBINED, 0, 0, 0, 1):
        case AC1(TEXEL0, 0, 0, 1):
    #endif
            gdp_combine_ac = gdp_combine_ac_1;
            break;
        case AC1(0, 0, 0, SHADE):
        case AC1(TEXEL0, 0, SHADE, 0):
    #ifndef APP_UNSM
        case AC1(SHADE, 0, TEXEL0, 0):
    #endif
        case AC2(TEXEL1, TEXEL0, LOD_FRACTION, TEXEL0, 0, 0, 0, SHADE):
            gdp_combine_ac = gdp_combine_ac_shade;
            break;
    #ifndef APP_UNSM
        case AC1(0, 0, 0, PRIMITIVE):
    #endif
        case AC1(TEXEL0, 0, PRIMITIVE, 0):
    #ifndef APP_UNSM
        case AC1(PRIMITIVE, 0, TEXEL0, 0):
    #endif
        case AC1(PRIMITIVE, SHADE, TEXEL0, SHADE): /*this is wrong*/
    #ifndef APP_UNSM
        case AC1(PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT): /*this is wrong*/
    #endif
            gdp_combine_ac = gdp_combine_ac_prim;
            break;
        case AC1(0, 0, 0, ENVIRONMENT):
        case AC1(TEXEL0, 0, ENVIRONMENT, 0):
        case AC1(ENVIRONMENT, 0, TEXEL0, 0):
            gdp_combine_ac = gdp_combine_ac_env;
            break;
        case AC1(SHADE, 0, ENVIRONMENT, 0):
            gdp_combine_ac = gdp_combine_ac_shade_env;
            break;
        default:
            gdp_combine_ac = gdp_combine_ac_0;
            wdebug("unknown ac %08" FMT_X "%08" FMT_X "\n", w0, w1);
            break;
    }
    gdp_set_tx(tx);
}
#undef CC
#undef AC
#undef CC1
#undef AC1
#undef CC2
#undef AC2

#define ZR  ((mode & Z_CMP) != 0)
#define ZW  ((mode & Z_UPD) != 0)
#define AC  ((mode & CVG_X_ALPHA) || (mode & 3) == G_AC_THRESHOLD)
#define BL  ((mode & 0x300) != CVG_DST_CLAMP && (mode & FORCE_BL))
#define DE  ((mode & 0xC00) == ZMODE_DEC)
static void gdp_set_rm(u32 mode)
{
#ifdef GEKKO
    GX_SetZMode(ZR, GX_LEQUAL, ZW);
    if (AC)
    {
        GX_SetAlphaCompare(GX_GEQUAL, 0x20, GX_AOP_AND, GX_ALWAYS, 0x00);
        GX_SetZCompLoc(GX_FALSE);
    }
    else
    {
        GX_SetAlphaCompare(GX_ALWAYS, 0x00, GX_AOP_AND, GX_ALWAYS, 0x00);
        GX_SetZCompLoc(GX_TRUE);
    }
    GX_SetBlendMode(
        BL ? GX_BM_BLEND : GX_BM_NONE, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA,
        GX_LO_NOOP
    );
    gsp_decal = DE;
    gsp_flush_mp();
#else
    if (ZR) glEnable(GL_DEPTH_TEST);
    else    glDisable(GL_DEPTH_TEST);
    glDepthMask(ZW);
    if (AC) glEnable(GL_ALPHA_TEST);
    else    glDisable(GL_ALPHA_TEST);
    if (BL) glEnable(GL_BLEND);
    else    glDisable(GL_BLEND);
    if (DE) glPolygonOffset(-1, -2);
    else    glPolygonOffset(0, 0);
#endif
}
#undef ZR
#undef ZW
#undef AC
#undef BL
#undef DE

static void gsp_start_rect(void)
{
    if (gdp_rect == 0)
    {
        f32 mf[4][4];
        mtxf_identity(mf);
        gsp_set_mm(mf);
        gsp_set_vp(0, 1280, 0, 960);
        gsp_set_cull(G_CULL_BACK);
        if (gdp_cycle) gdp_set_rm(G_OML_CYCLE);
    }
}

static void gsp_flush_rect(void)
{
    if (gdp_rect != 0)
    {
        gdp_rect = 0;
        gsp_flush_mp();
        gsp_flush_mm();
        gsp_flush_vp();
        gsp_flush_cull();
        gdp_flush_cc();
        gdp_flush_rm();
    }
}

static void gsp_fillrect(u32 w0, u32 w1)
{
    int xh = w0 >> 12 & 0x0FFF;
    int yh = w0 >>  0 & 0x0FFF;
    int xl = w1 >> 12 & 0x0FFF;
    int yl = w1 >>  0 & 0x0FFF;
    uint r = RGBA16_R(gdp_fill);
    uint g = RGBA16_G(gdp_fill);
    uint b = RGBA16_B(gdp_fill);
    uint i;
    gsp_start_rect();
    if (gdp_rect != 1)
    {
        f32 mf[4][4];
        gdp_rect = 1;
        mtxf_ortho(mf, 0, 1280, 960, 0, 0, 2);
        gsp_set_mp(mf);
        if (gdp_cycle) gdp_set_cc(0xFCFFFFFF, 0xFFFE793C);
    }
    if (gdp_cycle)
    {
        xh = (xh & ~3) + 4;
        yh = (yh & ~3) + 4;
    }
    for (i = 0; i < 4; i++)
    {
        struct vtx  *v  = &gsp_vtx_buf[i];
        struct vtxf *vf = &gsp_vtxf_buf[i];
        v->x = i == 0 || i == 3 ? xl : xh;
        v->y = i == 2 || i == 3 ? yl : yh;
        v->z = 0;
        vf->shade[0] = r;
        vf->shade[1] = g;
        vf->shade[2] = b;
        vf->shade[3] = 0xFF;
    }
    gdp_tri(gdp_rect_tri[0]);
    gdp_tri(gdp_rect_tri[1]);
}

static void gsp_texrect(void)
{
    int   xh   = (s16)(gdp_texrect[0] >> 8) >> 4;
    int   yh   = (s16)(gdp_texrect[0] << 4) >> 4;
    int   xl   = (s16)(gdp_texrect[1] >> 8) >> 4;
    int   yl   = (s16)(gdp_texrect[1] << 4) >> 4;
    float ul   = (1.0F/32)        * (s16)(gdp_texrect[2] >> 16);
    float vl   = (1.0F/32)        * (s16)(gdp_texrect[2] >>  0);
    float dsdx = (32/4.0F/0x0400) * (s16)(gdp_texrect[3] >> 16);
    float dtdy = (32/4.0F/0x0400) * (s16)(gdp_texrect[3] >>  0);
    float uh;
    float vh;
    uint  i;
    gsp_start_rect();
    if (gdp_rect != 2)
    {
        f32 mf[4][4];
        gdp_rect = 2;
        mtxf_ortho(
            mf,
        #ifdef LIB_DYNRES
            4*lib_video_l, 4*lib_video_r,
        #else
            0, 1280,
        #endif
            960, 0, 0, 2
        );
        gsp_set_mp(mf);
        if (gdp_cycle) gdp_set_cc(0xFC121824, 0xFF33FFFF);
    }
    if (gdp_cycle)
    {
        xh = (xh & ~3) + 4;
        yh = (yh & ~3) + 4;
        dsdx = 32/4.0F;
    }
    else
    {
        if (gdp_tf != GDP_TF_POINT)
        {
            ul += 32*0.5F;
            vl += 32*0.5F;
        }
    }
    uh = ul + dsdx*(xh-xl);
    vh = vl + dtdy*(yh-yl);
    if (gdp_texrect[0] >> 24 & 1)
    {
        gsp_vtxf_buf[0].s = uh;
        gsp_vtxf_buf[0].t = vl;
        gsp_vtxf_buf[2].s = ul;
        gsp_vtxf_buf[2].t = vh;
    }
    else
    {
        gsp_vtxf_buf[0].s = ul;
        gsp_vtxf_buf[0].t = vh;
        gsp_vtxf_buf[2].s = uh;
        gsp_vtxf_buf[2].t = vl;
    }
    gsp_vtxf_buf[1].s = uh;
    gsp_vtxf_buf[1].t = vh;
    gsp_vtxf_buf[3].s = ul;
    gsp_vtxf_buf[3].t = vl;
    for (i = 0; i < 4; i++)
    {
        struct vtx  *v  = &gsp_vtx_buf[i];
        struct vtxf *vf = &gsp_vtxf_buf[i];
        v->x = i == 0 || i == 3 ? xl : xh;
        v->y = i == 2 || i == 3 ? yl : yh;
        v->z = 0;
        vf->shade[0] = 0xFF;
        vf->shade[1] = 0xFF;
        vf->shade[2] = 0xFF;
        vf->shade[3] = 0xFF;
    }
    gdp_tri(gdp_rect_tri[0]);
    gdp_tri(gdp_rect_tri[1]);
}

static void gsp_start(void *ucode, u32 *dl);

#include "gsp/g_spnoop.c"

#ifdef GSP_F3D
#include "gsp/g_mtx.c"
#include "gsp/g_movemem.c"
#include "gsp/g_vtx.c"
#include "gsp/g_dl.c"
#ifdef GSP_F3DEX
#include "gsp/g_load_ucode.c"
#include "gsp/g_branch_z.c"
#include "gsp/g_tri2.c"
#include "gsp/g_modifyvtx.c"
#else
#include "gsp/g_rdphalf_cont.c"
#endif
#include "gsp/g_rdphalf_2.c"
#include "gsp/g_rdphalf_1.c"
#ifdef GSP_F3D_20D
#include "gsp/g_perspnormalize.c"
#endif
#ifdef GSP_F3DEX
#include "gsp/g_quad.c"
#endif
#include "gsp/g_cleargeometrymode.c"
#include "gsp/g_setgeometrymode.c"
#include "gsp/g_enddl.c"
#include "gsp/g_setothermode_l.c"
#include "gsp/g_setothermode_h.c"
#include "gsp/g_texture.c"
#include "gsp/g_moveword.c"
#include "gsp/g_popmtx.c"
#if 0
#include "gsp/g_culldl.c"
#else
#define gsp_g_culldl            NULL
#endif
#include "gsp/g_tri1.c"
#include "gdp/g_noop.c"
#endif

#ifdef GSP_F3DEX2
#include "gsp/g_vtx.c"
#include "gsp/g_modifyvtx.c"
#include "gsp/g_culldl.c"
#include "gsp/g_branch_z.c"
#include "gsp/g_tri1.c"
#include "gsp/g_tri2.c"

static void gsp_g_quad(unused u32 w0, unused u32 w1)
{
    puts("G_QUAD");
    exit(EXIT_FAILURE);
}

static void gsp_g_special_3(unused u32 w0, unused u32 w1)
{
    puts("G_SPECIAL_3");
    exit(EXIT_FAILURE);
}

static void gsp_g_special_2(unused u32 w0, unused u32 w1)
{
    puts("G_SPECIAL_2");
    exit(EXIT_FAILURE);
}

#include "gsp/g_special_1.c"

static void gsp_g_dma_io(unused u32 w0, unused u32 w1)
{
    puts("G_DMA_IO");
    exit(EXIT_FAILURE);
}

#include "gsp/g_texture.c"
#include "gsp/g_popmtx.c"
#include "gsp/g_geometrymode.c"
#include "gsp/g_mtx.c"
#include "gsp/g_moveword.c"
#include "gsp/g_movemem.c"
#include "gsp/g_load_ucode.c"
#include "gsp/g_dl.c"
#include "gsp/g_enddl.c"
#include "gdp/g_noop.c"
#include "gsp/g_rdphalf_1.c"
#include "gsp/g_setothermode_l.c"
#include "gsp/g_setothermode_h.c"
#include "gsp/g_rdphalf_2.c"

static void gsp_g_obj_rectangle(unused u32 w0, unused u32 w1)
{
    puts("G_OBJ_RECTANGLE");
    exit(EXIT_FAILURE);
}

static void gsp_g_obj_sprite(unused u32 w0, unused u32 w1)
{
    puts("G_OBJ_SPRITE");
    exit(EXIT_FAILURE);
}

static void gsp_g_select_dl(unused u32 w0, unused u32 w1)
{
    puts("G_SELECT_DL");
    exit(EXIT_FAILURE);
}

static void gsp_g_obj_loadtxtr(unused u32 w0, unused u32 w1)
{
    /*
    puts("G_OBJ_LOADTXTR");
    exit(EXIT_FAILURE);
    */
}

static void gsp_g_obj_ldtx_sprite(unused u32 w0, unused u32 w1)
{
    puts("G_OBJ_LDTX_SPRITE");
    exit(EXIT_FAILURE);
}

static void gsp_g_obj_ldtx_rect(unused u32 w0, unused u32 w1)
{
    puts("G_OBJ_LDTX_RECT");
    exit(EXIT_FAILURE);
}

static void gsp_g_obj_ldtx_rect_r(unused u32 w0, unused u32 w1)
{
    puts("G_OBJ_LDTX_RECT_R");
    exit(EXIT_FAILURE);
}

static void gsp_g_bg_1cyc(unused u32 w0, unused u32 w1)
{
    /*
    struct obj_bg *bg = gsp_addr(w1);
    printf(
        "image_x     = %f\n"
        "image_w     = %f\n"
        "frame_x     = %f\n"
        "frame_w     = %f\n"
        "image_y     = %f\n"
        "image_h     = %f\n"
        "frame_y     = %f\n"
        "frame_h     = %f\n"
        "image_ptr   = 0x%08" FMT_X "\n"
        "image_load  = 0x%04X\n"
        "image_fmt   = G_IM_FMT_%s\n"
        "image_siz   = G_IM_SIZ_%db\n"
        "image_pal   = 0x%04X\n"
        "image_flip  = 0x%04X\n"
        "scale_w     = %f\n"
        "scale_h     = %f\n",
        (1.0F/32) * bg->image_x,
        (1.0F/ 4) * bg->image_w,
        (1.0F/ 4) * bg->frame_x,
        (1.0F/ 4) * bg->frame_w,
        (1.0F/32) * bg->image_y,
        (1.0F/ 4) * bg->image_h,
        (1.0F/ 4) * bg->frame_y,
        (1.0F/ 4) * bg->frame_h,
        bg->image_ptr,
        bg->image_load,
        str_im_fmt[bg->image_fmt],
        4 << bg->image_siz,
        bg->image_pal,
        bg->image_flip,
        (1.0F/0x400) * bg->scale_w,
        (1.0F/0x400) * bg->scale_h
    );
    exit(EXIT_FAILURE);
    */
}

static void gsp_g_bg_copy(unused u32 w0, unused u32 w1)
{
    /*
    puts("G_BG_COPY");
    exit(EXIT_FAILURE);
    */
}

static void gsp_g_obj_rendermode(unused u32 w0, u32 w1)
{
    gsp_obj_rendermode = w1;
}

static void gsp_g_obj_rectangle_r(unused u32 w0, unused u32 w1)
{
    puts("G_OBJ_RECTANGLE_R");
    exit(EXIT_FAILURE);
}

static void gsp_g_obj_movemem(unused u32 w0, unused u32 w1)
{
    puts("G_OBJ_MOVEMEM");
    exit(EXIT_FAILURE);
}

static void gsp_g_rdphalf_0(unused u32 w0, unused u32 w1)
{
    /*
    puts("G_RDPHALF_0");
    exit(EXIT_FAILURE);
    */
}
#endif

#include "gdp/g_setcimg.c"
#include "gdp/g_setzimg.c"
#include "gdp/g_settimg.c"
#include "gdp/g_setcombine.c"
#include "gdp/g_setenvcolor.c"
#include "gdp/g_setprimcolor.c"
#include "gdp/g_setblendcolor.c"
#include "gdp/g_setfogcolor.c"
#include "gdp/g_setfillcolor.c"
#include "gdp/g_fillrect.c"
#include "gdp/g_settile.c"
#include "gdp/g_loadtile.c"
#include "gdp/g_loadblock.c"
#include "gdp/g_settilesize.c"
#include "gdp/g_loadtlut.c"
#include "gdp/g_rdpsetothermode.c"
#include "gdp/g_setprimdepth.c"
#include "gdp/g_setscissor.c"
#include "gdp/g_setconvert.c"
#include "gdp/g_setkeyr.c"
#include "gdp/g_setkeygb.c"
#include "gdp/g_rdpfullsync.c"
#include "gdp/g_rdptilesync.c"
#include "gdp/g_rdppipesync.c"
#include "gdp/g_rdploadsync.c"
#include "gdp/g_texrectflip.c"
#include "gdp/g_texrect.c"

static GSP *gsp_table[] =
{
    /* 0x00 */  gsp_g_spnoop,
#ifdef GSP_F3D
    /* 0x01 */  gsp_g_mtx,
    /* 0x02 */  NULL,
    /* 0x03 */  gsp_g_movemem,
    /* 0x04 */  gsp_g_vtx,
    /* 0x05 */  NULL,
    /* 0x06 */  gsp_g_dl,
#endif
#ifdef GSP_F3DEX2
    /* 0x01 */  NULL,
    /* 0x02 */  NULL,
    /* 0x03 */  NULL,
    /* 0x04 */  NULL,
    /* 0x05 */  NULL,
    /* 0x06 */  NULL,
#endif
    /* 0x07 */  NULL,
    /* 0x08 */  NULL,
    /* 0x09 */  NULL,
    /* 0x0A */  NULL,
    /* 0x0B */  NULL,
    /* 0x0C */  NULL,
    /* 0x0D */  NULL,
    /* 0x0E */  NULL,
    /* 0x0F */  NULL,
    /* 0x10 */  NULL,
    /* 0x11 */  NULL,
    /* 0x12 */  NULL,
    /* 0x13 */  NULL,
    /* 0x14 */  NULL,
    /* 0x15 */  NULL,
    /* 0x16 */  NULL,
    /* 0x17 */  NULL,
    /* 0x18 */  NULL,
    /* 0x19 */  NULL,
    /* 0x1A */  NULL,
    /* 0x1B */  NULL,
    /* 0x1C */  NULL,
    /* 0x1D */  NULL,
    /* 0x1E */  NULL,
    /* 0x1F */  NULL,
    /* 0x20 */  NULL,
    /* 0x21 */  NULL,
    /* 0x22 */  NULL,
    /* 0x23 */  NULL,
    /* 0x24 */  NULL,
    /* 0x25 */  NULL,
    /* 0x26 */  NULL,
    /* 0x27 */  NULL,
    /* 0x28 */  NULL,
    /* 0x29 */  NULL,
    /* 0x2A */  NULL,
    /* 0x2B */  NULL,
    /* 0x2C */  NULL,
    /* 0x2D */  NULL,
    /* 0x2E */  NULL,
    /* 0x2F */  NULL,
    /* 0x30 */  NULL,
    /* 0x31 */  NULL,
    /* 0x32 */  NULL,
    /* 0x33 */  NULL,
    /* 0x34 */  NULL,
    /* 0x35 */  NULL,
    /* 0x36 */  NULL,
    /* 0x37 */  NULL,
    /* 0x38 */  NULL,
    /* 0x39 */  NULL,
    /* 0x3A */  NULL,
    /* 0x3B */  NULL,
    /* 0x3C */  NULL,
    /* 0x3D */  NULL,
    /* 0x3E */  NULL,
    /* 0x3F */  NULL,
    /* 0x40 */  NULL,
    /* 0x41 */  NULL,
    /* 0x42 */  NULL,
    /* 0x43 */  NULL,
    /* 0x44 */  NULL,
    /* 0x45 */  NULL,
    /* 0x46 */  NULL,
    /* 0x47 */  NULL,
    /* 0x48 */  NULL,
    /* 0x49 */  NULL,
    /* 0x4A */  NULL,
    /* 0x4B */  NULL,
    /* 0x4C */  NULL,
    /* 0x4D */  NULL,
    /* 0x4E */  NULL,
    /* 0x4F */  NULL,
    /* 0x50 */  NULL,
    /* 0x51 */  NULL,
    /* 0x52 */  NULL,
    /* 0x53 */  NULL,
    /* 0x54 */  NULL,
    /* 0x55 */  NULL,
    /* 0x56 */  NULL,
    /* 0x57 */  NULL,
    /* 0x58 */  NULL,
    /* 0x59 */  NULL,
    /* 0x5A */  NULL,
    /* 0x5B */  NULL,
    /* 0x5C */  NULL,
    /* 0x5D */  NULL,
    /* 0x5E */  NULL,
    /* 0x5F */  NULL,
    /* 0x60 */  NULL,
    /* 0x61 */  NULL,
    /* 0x62 */  NULL,
    /* 0x63 */  NULL,
    /* 0x64 */  NULL,
    /* 0x65 */  NULL,
    /* 0x66 */  NULL,
    /* 0x67 */  NULL,
    /* 0x68 */  NULL,
    /* 0x69 */  NULL,
    /* 0x6A */  NULL,
    /* 0x6B */  NULL,
    /* 0x6C */  NULL,
    /* 0x6D */  NULL,
    /* 0x6E */  NULL,
    /* 0x6F */  NULL,
    /* 0x70 */  NULL,
    /* 0x71 */  NULL,
    /* 0x72 */  NULL,
    /* 0x73 */  NULL,
    /* 0x74 */  NULL,
    /* 0x75 */  NULL,
    /* 0x76 */  NULL,
    /* 0x77 */  NULL,
    /* 0x78 */  NULL,
    /* 0x79 */  NULL,
    /* 0x7A */  NULL,
    /* 0x7B */  NULL,
    /* 0x7C */  NULL,
    /* 0x7D */  NULL,
    /* 0x7E */  NULL,
    /* 0x7F */  NULL,
    /* 0x80 */  NULL,
    /* 0x81 */  NULL,
    /* 0x82 */  NULL,
    /* 0x83 */  NULL,
    /* 0x84 */  NULL,
    /* 0x85 */  NULL,
    /* 0x86 */  NULL,
    /* 0x87 */  NULL,
    /* 0x88 */  NULL,
    /* 0x89 */  NULL,
    /* 0x8A */  NULL,
    /* 0x8B */  NULL,
    /* 0x8C */  NULL,
    /* 0x8D */  NULL,
    /* 0x8E */  NULL,
    /* 0x8F */  NULL,
    /* 0x90 */  NULL,
    /* 0x91 */  NULL,
    /* 0x92 */  NULL,
    /* 0x93 */  NULL,
    /* 0x94 */  NULL,
    /* 0x95 */  NULL,
    /* 0x96 */  NULL,
    /* 0x97 */  NULL,
    /* 0x98 */  NULL,
    /* 0x99 */  NULL,
    /* 0x9A */  NULL,
    /* 0x9B */  NULL,
    /* 0x9C */  NULL,
    /* 0x9D */  NULL,
    /* 0x9E */  NULL,
    /* 0x9F */  NULL,
    /* 0xA0 */  NULL,
    /* 0xA1 */  NULL,
    /* 0xA2 */  NULL,
    /* 0xA3 */  NULL,
    /* 0xA4 */  NULL,
    /* 0xA5 */  NULL,
    /* 0xA6 */  NULL,
    /* 0xA7 */  NULL,
    /* 0xA8 */  NULL,
    /* 0xA9 */  NULL,
    /* 0xAA */  NULL,
    /* 0xAB */  NULL,
    /* 0xAC */  NULL,
    /* 0xAD */  NULL,
    /* 0xAE */  NULL,
#ifdef GSP_F3D
#ifdef GSP_F3DEX
    /* 0xAF */  gsp_g_load_ucode,
    /* 0xB0 */  gsp_g_branch_z,
    /* 0xB1 */  gsp_g_tri2,
    /* 0xB2 */  gsp_g_modifyvtx,
    /* 0xB3 */  gsp_g_rdphalf_2,
    /* 0xB4 */  gsp_g_rdphalf_1,
    /* 0xB5 */  gsp_g_quad,
#else
    /* 0xAF */  NULL,
    /* 0xB0 */  NULL,
#ifdef GSP_F3D_20D
    /* 0xB1 */  gsp_g_rdphalf_cont,
    /* 0xB2 */  gsp_g_rdphalf_2,
    /* 0xB3 */  gsp_g_rdphalf_1,
    /* 0xB4 */  gsp_g_perspnormalize,
#else
    /* 0xB1 */  NULL,
    /* 0xB2 */  gsp_g_rdphalf_cont,
    /* 0xB3 */  gsp_g_rdphalf_2,
    /* 0xB4 */  gsp_g_rdphalf_1,
#endif
    /* 0xB5 */  NULL,
#endif
    /* 0xB6 */  gsp_g_cleargeometrymode,
    /* 0xB7 */  gsp_g_setgeometrymode,
    /* 0xB8 */  gsp_g_enddl,
    /* 0xB9 */  gsp_g_setothermode_l,
    /* 0xBA */  gsp_g_setothermode_h,
    /* 0xBB */  gsp_g_texture,
    /* 0xBC */  gsp_g_moveword,
    /* 0xBD */  gsp_g_popmtx,
    /* 0xBE */  gsp_g_culldl,
    /* 0xBF */  gsp_g_tri1,
    /* 0xC0 */  gdp_g_noop,
#endif
#ifdef GSP_F3DEX2
    /* 0xAF */  NULL,
    /* 0xB0 */  NULL,
    /* 0xB1 */  NULL,
    /* 0xB2 */  NULL,
    /* 0xB3 */  NULL,
    /* 0xB4 */  NULL,
    /* 0xB5 */  NULL,
    /* 0xB6 */  NULL,
    /* 0xB7 */  NULL,
    /* 0xB8 */  NULL,
    /* 0xB9 */  NULL,
    /* 0xBA */  NULL,
    /* 0xBB */  NULL,
    /* 0xBC */  NULL,
    /* 0xBD */  NULL,
    /* 0xBE */  NULL,
    /* 0xBF */  NULL,
    /* 0xC0 */  NULL,
#endif
    /* 0xC1 */  NULL,
    /* 0xC2 */  NULL,
    /* 0xC3 */  NULL,
    /* 0xC4 */  NULL,
    /* 0xC5 */  NULL,
    /* 0xC6 */  NULL,
    /* 0xC7 */  NULL,
    /* 0xC8 */  NULL,
    /* 0xC9 */  NULL,
    /* 0xCA */  NULL,
    /* 0xCB */  NULL,
    /* 0xCC */  NULL,
    /* 0xCD */  NULL,
    /* 0xCE */  NULL,
    /* 0xCF */  NULL,
    /* 0xD0 */  NULL,
    /* 0xD1 */  NULL,
    /* 0xD2 */  NULL,
#ifdef GSP_F3D
    /* 0xD3 */  NULL,
    /* 0xD4 */  NULL,
    /* 0xD5 */  NULL,
    /* 0xD6 */  NULL,
    /* 0xD7 */  NULL,
    /* 0xD8 */  NULL,
    /* 0xD9 */  NULL,
    /* 0xDA */  NULL,
    /* 0xDB */  NULL,
    /* 0xDC */  NULL,
    /* 0xDD */  NULL,
    /* 0xDE */  NULL,
    /* 0xDF */  NULL,
    /* 0xE0 */  NULL,
    /* 0xE1 */  NULL,
    /* 0xE2 */  NULL,
    /* 0xE3 */  NULL,
#endif
#ifdef GSP_F3DEX2
    /* 0xD3 */  gsp_g_special_3,
    /* 0xD4 */  gsp_g_special_2,
    /* 0xD5 */  gsp_g_special_1,
    /* 0xD6 */  gsp_g_dma_io,
    /* 0xD7 */  gsp_g_texture,
    /* 0xD8 */  gsp_g_popmtx,
    /* 0xD9 */  gsp_g_geometrymode,
    /* 0xDA */  gsp_g_mtx,
    /* 0xDB */  gsp_g_moveword,
    /* 0xDC */  gsp_g_movemem,
    /* 0xDD */  gsp_g_load_ucode,
    /* 0xDE */  gsp_g_dl,
    /* 0xDF */  gsp_g_enddl,
    /* 0xE0 */  gdp_g_noop,
    /* 0xE1 */  gsp_g_rdphalf_1,
    /* 0xE2 */  gsp_g_setothermode_l,
    /* 0xE3 */  gsp_g_setothermode_h,
#endif
    /* 0xE4 */  gdp_g_texrect,
    /* 0xE5 */  gdp_g_texrectflip,
    /* 0xE6 */  gdp_g_rdploadsync,
    /* 0xE7 */  gdp_g_rdppipesync,
    /* 0xE8 */  gdp_g_rdptilesync,
    /* 0xE9 */  gdp_g_rdpfullsync,
    /* 0xEA */  gdp_g_setkeygb,
    /* 0xEB */  gdp_g_setkeyr,
    /* 0xEC */  gdp_g_setconvert,
    /* 0xED */  gdp_g_setscissor,
    /* 0xEE */  gdp_g_setprimdepth,
    /* 0xEF */  gdp_g_rdpsetothermode,
    /* 0xF0 */  gdp_g_loadtlut,
#ifdef GSP_F3DEX2
    /* 0xF1 */  gsp_g_rdphalf_2,
#else
    /* 0xF1 */  NULL,
#endif
    /* 0xF2 */  gdp_g_settilesize,
    /* 0xF3 */  gdp_g_loadblock,
    /* 0xF4 */  gdp_g_loadtile,
    /* 0xF5 */  gdp_g_settile,
    /* 0xF6 */  gdp_g_fillrect,
    /* 0xF7 */  gdp_g_setfillcolor,
    /* 0xF8 */  gdp_g_setfogcolor,
    /* 0xF9 */  gdp_g_setblendcolor,
    /* 0xFA */  gdp_g_setprimcolor,
    /* 0xFB */  gdp_g_setenvcolor,
    /* 0xFC */  gdp_g_setcombine,
    /* 0xFD */  gdp_g_settimg,
    /* 0xFE */  gdp_g_setzimg,
    /* 0xFF */  gdp_g_setcimg,
};

#ifdef GSP_F3DEX2
static GSP *const gsp_table_3d[] =
{
    /* 0x01 */  gsp_g_vtx,
    /* 0x02 */  gsp_g_modifyvtx,
    /* 0x03 */  gsp_g_culldl,
    /* 0x04 */  gsp_g_branch_z,
    /* 0x05 */  gsp_g_tri1,
    /* 0x06 */  gsp_g_tri2,
    /* 0x07 */  gsp_g_quad,
    /* 0x08 */  NULL,
    /* 0x09 */  NULL,
    /* 0x0A */  NULL,
    /* 0x0B */  NULL,
};

static GSP *const gsp_table_2d[] =
{
    /* 0x01 */  gsp_g_obj_rectangle,
    /* 0x02 */  gsp_g_obj_sprite,
    /* 0x03 */  gsp_g_culldl,
    /* 0x04 */  gsp_g_select_dl,
    /* 0x05 */  gsp_g_obj_loadtxtr,
    /* 0x06 */  gsp_g_obj_ldtx_sprite,
    /* 0x07 */  gsp_g_obj_ldtx_rect,
    /* 0x08 */  gsp_g_obj_ldtx_rect_r,
    /* 0x09 */  gsp_g_bg_1cyc,
    /* 0x0A */  gsp_g_bg_copy,
    /* 0x0B */  gsp_g_obj_rendermode,
};
#endif

void gsp_init(void)
{
#ifdef GEKKO
    GX_ClearVtxDesc();
    GX_SetCurrentMtx(GX_PNMTX0);
    GX_SetVtxDesc(GX_VA_POS,  GX_DIRECT);
    GX_SetVtxDesc(GX_VA_CLR0, GX_DIRECT);
    GX_SetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS,  GX_POS_XYZ,  GX_S16,   0);
    GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
    GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST,   GX_F32,   0);
    GX_SetNumChans(1);
    GX_SetNumTexGens(1);
#else
#ifdef __3DS__
    pglInit();
#endif
    glEnable(GL_SCISSOR_TEST);
    glEnable(GL_POLYGON_OFFSET_FILL);
#ifdef __3DS__
    glDepthFunc(GL_LESS);
#endif
    glAlphaFunc(GL_GEQUAL, 1.0F/8);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
#ifndef GSP_SWFOG
    glFogi(GL_FOG_MODE, GL_LINEAR);
    glFogi(GL_FOG_COORD_SRC, GL_FOG_COORD);
    glFogf(GL_FOG_START, 0x00);
    glFogf(GL_FOG_END,   0xFF);
#endif
#endif
    gdp_txtcache = NULL;
    gsp_new_cache = false;
}

void gsp_exit(void)
{
#ifdef __3DS__
    pglExit();
#endif
}

void gsp_cache(void)
{
    gsp_new_cache = true;
}

static void gsp_start(void *ucode, u32 *dl)
{
    u32 i;
#ifdef GSP_F3DEX
    i = (u8 *)ucode - cpu_dram;
    switch (i)
    {
    #ifdef APP_UNKT
    #ifdef APP_E0
        /* F3DEX */
        case 0x000D9040:
            break;
        /* F3DLX */
        case 0x000DA420:
            break;
    #endif
    #endif
    #ifdef APP_UNK4
    #ifdef APP_E0
        case 0x00039E90:
            /* pdebug("notice: using F3DEX2\n"); */
            goto meme;
        case 0x0003B220:
            pdebug("notice: using L3DEX2\n");
        meme:
            memcpy(&gsp_table[0x01], gsp_table_3d, sizeof(gsp_table_3d));
            gsp_table[G_MTX]     = gsp_g_mtx;
            gsp_table[G_MOVEMEM] = gsp_g_movemem;
            gsp_table[G_TEXRECT] = gdp_g_texrect;
            break;
        case 0x0003C3B0:
            /* pdebug("notice: using S2DEX2\n"); */
            memcpy(&gsp_table[0x01], gsp_table_2d, sizeof(gsp_table_2d));
            gsp_table[G_OBJ_RECTANGLE_R] = gsp_g_obj_rectangle_r;
            gsp_table[G_OBJ_MOVEMEM]     = gsp_g_obj_movemem;
            gsp_table[G_RDPHALF_0]       = gsp_g_rdphalf_0;
            break;
    #endif
    #endif
        default:
            eprint("unknown ucode 0x%08" FMT_X "\n", i);
            break;
    }
#else
    (void)ucode;
#endif
    gdp_triangle = gdp_tri;
    gdp_combine_cc = gdp_combine_cc_0;
    gdp_combine_ac = gdp_combine_ac_0;
    gdp_rect = 0;
    gsp_mtxf_modelview = gsp_mtxf_modelview_stack;
    for (i = 0; i < lenof(gsp_addr_table); i++) gsp_addr_table[i] = cpu_dram;
    gsp_dl_stack[0] = dl;
    gsp_dl_index = 0;
    gsp_lookat = false;
    gsp_new_texture = false;
    gsp_new_fog     = false;
    gsp_new_light   = false;
}

static void gsp_draw(void *ucode, u32 *dl)
{
#ifdef GEKKO
    GX_InvVtxCache();
    GX_SetViewport(0, 0, lib_video_w, lib_video_h, 0, 1);
#else
    glViewport(0, 0, lib_video_w, lib_video_h);
    glScissor(0, 0, lib_video_w, lib_video_h);
    glDepthMask(GL_TRUE);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
#endif
    gsp_start(ucode, dl);
    do
    {
        void (*cmd)(u32, u32);
        u32 w0 = gsp_dl_stack[gsp_dl_index][0];
        u32 w1 = gsp_dl_stack[gsp_dl_index][1];
    #if 0
        printf(
            "0x%08X: %08X %08X\n",
            (uint)((u8 *)gsp_dl_stack[gsp_dl_index]-cpu_dram), w0, w1
        );
    #endif
        gsp_dl_stack[gsp_dl_index] += 2;
        cmd = gsp_table[w0 >> 24];
        if (cmd != NULL)
        {
            cmd(w0, w1);
        }
        else
        {
            edebug("invalid Gfx {{0x%08" FMT_X ", 0x%08" FMT_X "}}\n", w0, w1);
        }
    }
    while (gsp_dl_index >= 0);
#ifdef __3DS__
    pglSwapBuffers();
#endif
#ifdef GEKKO
    GX_DrawDone();
#endif
}

void gsp_update(void *ucode, u32 *dl)
{
#ifdef __3DS__
    float depth;
#endif
#ifdef GSP_CACHE
    if (gsp_new_cache)
#endif
    {
        gsp_new_cache = false;
        while (gdp_txtcache != NULL)
        {
            struct txtcache *txtcache = gdp_txtcache;
        #ifdef GEKKO
            free(txtcache->buf);
        #else
            glDeleteTextures(1, &txtcache->name);
        #endif
            gdp_txtcache = txtcache->next;
            free(txtcache);
        }
    }
#ifdef __3DS__
    depth = gfxIsWide() ? 0 : (1.0F/3)*osGet3DSliderState();
    pglSelectScreen(GFX_TOP, GFX_LEFT);
    gsp_depth = depth;
#endif
    gsp_draw(ucode, dl);
#ifdef __3DS__
    if (depth > 0)
    {
        pglSwapBuffers();
        pglSelectScreen(GFX_TOP, GFX_RIGHT);
        gsp_depth = -depth;
        gsp_draw(ucode, dl);
        pglSwapBuffers();
    }
#endif
}
#endif
