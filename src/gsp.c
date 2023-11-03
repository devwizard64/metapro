#include "types.h"
#include "app.h"
#include "sys.h"
#include "mtx.h"
#include "cpu.h"

/* #define GSP_DEBUG */

#if defined(GSP_F3D) || defined(GSP_F3DEX2)

#ifdef WIN32
#define GSP_SWFOG
#endif
#ifdef __3DS__
#define GSP_SWFOG
#endif

#ifdef __NATIVE__
extern void glFogCoordf(GLfloat);
#endif

#include "gbi.h"

#ifdef GSP_F3DEX
#define GSP_GFX_LEN     18
#define GSP_VTX_LEN     32
#define GSP_TRI_LEN     2
#else
#define GSP_GFX_LEN     10
#define GSP_VTX_LEN     16
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

#define MP gsp_mtx_projection
#define MM (*gsp_mtx_modelview)

#if defined(__NATIVE__) || defined(__3DS__)
#define GDP_TF_POINT    GL_NEAREST
#define GDP_TF_BILERP   GL_LINEAR
#define GDP_TX_NULL     -1
#define GDP_TX_MODULATE GL_MODULATE
#define GDP_TX_DECAL    GL_DECAL
#define GDP_TX_BLEND    GL_BLEND
typedef GLuint GDP_TF;
typedef GLint  GDP_TX;
typedef struct txtarg
{
    GLint internalFormat;
#ifdef __NATIVE__
    GLenum format;
    GLenum type;
#endif
}
TXTARG;
#endif
#ifdef GEKKO
#define GDP_TF_POINT    GX_NEAR
#define GDP_TF_BILERP   GX_LINEAR
#define GDP_TX_NULL     GX_PASSCLR
#define GDP_TX_MODULATE GX_MODULATE
#define GDP_TX_DECAL    GX_DECAL
#define GDP_TX_BLEND    GX_BLEND
typedef u8 GDP_TF;
typedef u8 GDP_TX;
typedef struct txtarg
{
    u8 fmt;
}
TXTARG;
#endif
#ifdef __NDS__
#define GDP_TF_POINT    0
#define GDP_TF_BILERP   1
#define GDP_TX_NULL     -1
#define GDP_TX_MODULATE POLY_MODULATION
#define GDP_TX_DECAL    POLY_DECAL
#define GDP_TX_BLEND    POLY_MODULATION
typedef u8  GDP_TF;
typedef int GDP_TX;
typedef struct txtarg
{
    u8 type;
}
TXTARG;
#endif

typedef struct tile
{
    u16 line;
    u16 tmem;
    u8  pal;
    u8  fmt;
    u8  cm[2];
    u8  mask[2];
    u8  shift[2];
    u16 ul[2];
    u16 lr[2];
}
TILE;

typedef struct txtcache
{
    struct txtcache *next;
    u8 *timg;
    u8 fmt;
    GDP_TF tf;
#if defined(__NATIVE__) || defined(__3DS__)
    GLuint name;
#endif
#ifdef GEKKO
    u8 *buf;
    GXTexObj obj;
#endif
#ifdef __NDS__
    int name;
#endif
}
TXTCACHE;

typedef struct vtxf
{
#ifdef GSP_SWVTX
    f32 x;
    f32 y;
    f32 z;
#endif
    f32 s;
    f32 t;
    u8  shade[4];
}
VTXF;

typedef struct lightf
{
    f32 r;
    f32 g;
    f32 b;
    f32 x;
    f32 y;
    f32 z;
}
LIGHTF;

typedef void GSPCALL(u32 w0, u32 w1);
typedef void *GDP_TEXTURE(TXTARG *arg, const u8 *src, uint w, uint h);
typedef void GDP_COMBINE(u8 *col, VTXF *vf);
typedef void GDP_TRIANGLE(const u8 *t);

static GDP_COMBINE *gdp_combine_cc;
static GDP_COMBINE *gdp_combine_ac;
static GDP_TRIANGLE *gdp_triangle;
static TXTCACHE *gdp_txtcache;
static GDP_TF gdp_tf;

static u8 gdp_tmem[0x2000];
static TILE gdp_tile[8];
static u16 *gdp_cimg;
static u8  *gdp_timg;
static u16  gdp_timg_w;

static u8  gdp_env[4];
static u8  gdp_prim[4];
#if 0
static u8  gdp_blend[4];
#endif
static u8  gdp_fog[4];
static u16 gdp_fill;

static u32 gdp_texrect[4];

static u16 gdp_texture_size[2];
static f32 gdp_texture_scale[2];

static u32 gdp_combine_w0;
static u32 gdp_combine_w1;
static u32 gdp_othermode_h;
static u32 gdp_othermode_l;
static u32 gdp_cycle;
static u8  gdp_rect;

static VP     gsp_viewport;
static LIGHT  gsp_light_buf[10];
static LIGHTF gsp_lightf_buf[10];
static VTX    gsp_vtx_buf[GSP_VTX_LEN+4];
static VTXF   gsp_vtxf_buf[GSP_VTX_LEN+4];
#ifdef APP_UNK4
static s16  gsp_mtx[32];
#endif
static f32  MP[4][4];
static f32  gsp_mtx_modelview_stack[16][4][4];
#if defined(GSP_SWFOG) || defined(__NATIVE__)
static f32  gsp_mtx_mvp[4][4];
#endif
static f32  MM[4][4];
static u8  *gsp_addr_table[16];
static u32 *gsp_dl_stack[GSP_GFX_LEN];
static s8   gsp_dl_index;
static u16  gsp_texture_scale[2];
static u32  gsp_geometry_mode;
static s16  gsp_fog_m;
static s16  gsp_fog_o;
static u8   gsp_light_no;
static bool gsp_new_cache;
static bool gsp_new_texture;
static bool gsp_new_fog;
static bool gsp_new_light;

#ifdef GSP_F3DEX2
static u8 gsp_obj_rendermode;
#endif

#ifdef GEKKO
static bool gsp_decal;
#endif
#ifdef __NDS__
static int gsp_polyfmt;
#endif

#ifndef __NDS__
static const u8 gdp_rect_tri[][3] =
{
    {GSP_VTX_LEN+0, GSP_VTX_LEN+1, GSP_VTX_LEN+2},
    {GSP_VTX_LEN+0, GSP_VTX_LEN+2, GSP_VTX_LEN+3},
};
#endif

#ifdef GSP_DEBUG
static const char *const str_im_fmt[] = {"RGBA", "YUV", "CI", "IA", "I"};
#endif

static void *gsp_addr(PTR addr)
{
    return &gsp_addr_table[addr >> 24 & 0xF][addr & 0xFFFFFF];
}

#ifdef __NATIVE__
#include "gdp/texture.native.c"
#endif
#ifdef GEKKO
#include "gdp/texture.gekko.c"
#endif
#ifdef __NDS__
#include "gdp/texture.nds.c"
#endif
#ifdef __3DS__
#include "gdp/texture.3ds.c"
#endif

#ifdef APP_UNSM
#define gdp_texture_ci4    NULL
#define gdp_texture_ci8    NULL
#else
static void *gdp_texture_ci(TXTARG *arg, u8 *src, uint w, uint h)
{
    void *dst;
    if ((gdp_othermode_h & (1 << G_MDSFT_TEXTLUT)) == 0)
    {
        dst = gdp_texture_rgba16(arg, src, w, h);
    }
    else
    {
        dst = gdp_texture_ia16(arg, src, w, h);
    }
    free(src);
    return dst;
}

static void *gdp_texture_ci4(TXTARG *arg, const u8 *src, uint w, uint h)
{
    void *buf = malloc(2*w*h);
    u16 *tlut = (u16 *)&gdp_tmem[0x800 + 0x80*gdp_tile[0].pal];
    u16 *dst = buf;
    uint len = w*h/2;
    do
    {
        dst[0] = tlut[src[0] >> 4];
        dst[1] = tlut[src[0] & 15];
        dst += 2;
        src += 1;
        len -= 1;
    }
    while (len > 0);
    return gdp_texture_ci(arg, buf, w, h);
}

static void *gdp_texture_ci8(TXTARG *arg, const u8 *src, uint w, uint h)
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
    return gdp_texture_ci(arg, buf, w, h);
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

static const f32 gdp_texture_shift_table[] =
{
    /*  0 */ 1.0F / (32 <<  0),
    /*  1 */ 1.0F / (32 <<  1),
    /*  2 */ 1.0F / (32 <<  2),
    /*  3 */ 1.0F / (32 <<  3),
    /*  4 */ 1.0F / (32 <<  4),
    /*  5 */ 1.0F / (32 <<  5),
    /*  6 */ 1.0F / (32 <<  6),
    /*  7 */ 1.0F / (32 <<  7),
    /*  8 */ 1.0F / (32 <<  8),
    /*  9 */ 1.0F / (32 <<  9),
    /* 10 */ 1.0F / (32 << 10),
    /* -5 */ 1.0F / (32 >>  5),
    /* -4 */ 1.0F / (32 >>  4),
    /* -3 */ 1.0F / (32 >>  3),
    /* -2 */ 1.0F / (32 >>  2),
    /* -1 */ 1.0F / (32 >>  1),
};

#if defined(__NATIVE__) || defined(__3DS__)
static const GLuint gdp_texture_cm_table[] =
{
    /* 0x00 G_TX_NOMIRROR | G_TX_WRAP  */ GL_REPEAT,
    /* 0x01 G_TX_MIRROR   | G_TX_WRAP  */ GL_MIRRORED_REPEAT,
    /* 0x02 G_TX_NOMIRROR | G_TX_CLAMP */ GL_CLAMP_TO_EDGE,
    /* 0x03 G_TX_MIRROR   | G_TX_CLAMP */ GL_MIRRORED_REPEAT,
};
#endif
#ifdef GEKKO
static const u8 gdp_texture_cm_table[] =
{
    /* 0x00 G_TX_NOMIRROR | G_TX_WRAP  */ GX_REPEAT,
    /* 0x01 G_TX_MIRROR   | G_TX_WRAP  */ GX_MIRROR,
    /* 0x02 G_TX_NOMIRROR | G_TX_CLAMP */ GX_CLAMP,
    /* 0x03 G_TX_MIRROR   | G_TX_CLAMP */ GX_MIRROR,
};
#endif
#ifdef __NDS__
static int gdp_texture_sizeno(int x)
{
    if (x <=   8) return TEXTURE_SIZE_8;
    if (x <=  16) return TEXTURE_SIZE_16;
    if (x <=  32) return TEXTURE_SIZE_32;
    if (x <=  64) return TEXTURE_SIZE_64;
    if (x <= 128) return TEXTURE_SIZE_128;
    if (x <= 256) return TEXTURE_SIZE_256;
    if (x <= 512) return TEXTURE_SIZE_512;
    return TEXTURE_SIZE_1024;
}

static void *gdp_texture_resize(
    void *buf, UNUSED TXTARG *arg, uint dw, uint dh, uint sw, uint sh
)
{
    void *data = malloc(2*dw*dh);
    u16  *dst = data;
    u16  *src = buf;
    uint y = 0;
    do
    {
        uint x = 0;
        do
        {
            *dst++ = *src++;
            x++;
        }
        while (x < sw);
        do
        {
            *dst++ = 0;
            x++;
        }
        while (x < dw);
    }
    while (y < sh);
    do
    {
        uint x = 0;
        do
        {
            *dst++ = 0;
            x++;
        }
        while (x < dw);
    }
    while (y < dh);
    free(buf);
    return data;
}
#endif

static void gdp_set_texture(
    void *timg, void *src, uint fmt, GDP_TF tf, uint w, uint h,
    uint cms, uint cmt
)
{
    TXTCACHE *tc;
    GDP_TEXTURE *texture;
    for (tc = gdp_txtcache; tc != NULL; tc = tc->next)
    {
#ifdef __NDS__
        if (tc->timg == timg && tc->fmt == fmt)
#else
        if (tc->timg == timg && tc->fmt == fmt && tc->tf == tf)
#endif
        {
#if defined(__NATIVE__) || defined(__NDS__) || defined(__3DS__)
            glBindTexture(GL_TEXTURE_2D, tc->name);
#endif
#ifdef GEKKO
            GX_InvalidateTexAll();
            GX_LoadTexObj(&tc->obj, GX_TEXMAP0);
#endif
            return;
        }
    }
    texture = gdp_texture_table[fmt];
    if (texture != NULL)
    {
#if defined(__NATIVE__) || defined(__NDS__) || defined(__3DS__)
        u8 *buf;
#ifdef __NDS__
        int sizex;
        int sizey;
        int param;
#endif
#endif
        TXTARG arg;
        tc = malloc(sizeof(TXTCACHE));
        tc->next = gdp_txtcache;
        gdp_txtcache = tc;
        tc->timg = timg;
        tc->fmt  = fmt;
        tc->tf   = tf;
#if defined(APP_UNSM) && defined(APP_J00)
        if (tc->timg >= &cpu_dram[0x208100] && tc->timg < &cpu_dram[0x2211A0])
        {
            gsp_new_cache = TRUE;
        }
#endif
#if defined(__NATIVE__) || defined(__NDS__) || defined(__3DS__)
        buf = texture(&arg, src, w, h);
#ifndef __NDS__
        glFlush();
#endif
        glGenTextures(1, &tc->name);
        glBindTexture(GL_TEXTURE_2D, tc->name);
#ifdef __NDS__
        sizex = gdp_texture_sizeno(w);
        sizey = gdp_texture_sizeno(h);
        if (8U << sizex != w || 8U << sizey != h)
        {
            buf = gdp_texture_resize(buf, &arg, 8U << sizex, 8U << sizey, w, h);
        }
        param = TEXGEN_OFF;
        if (!(cms & G_TX_CLAMP ))   param |= GL_TEXTURE_WRAP_S;
        if (!(cmt & G_TX_CLAMP ))   param |= GL_TEXTURE_WRAP_T;
        if ( (cms & G_TX_MIRROR))   param |= GL_TEXTURE_FLIP_S;
        if ( (cmt & G_TX_MIRROR))   param |= GL_TEXTURE_FLIP_T;
        glTexImage2D(GL_TEXTURE_2D, 0, arg.type, sizex, sizey, 0, param, buf);
#else
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gdp_tf);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gdp_tf);
        glTexParameteri(
            GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, gdp_texture_cm_table[cms]
        );
        glTexParameteri(
            GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, gdp_texture_cm_table[cmt]
        );
#ifdef __NATIVE__
        glTexImage2D(
            GL_TEXTURE_2D, 0, arg.internalFormat, w, h, 0, arg.format, arg.type,
            buf
        );
#endif
#ifdef __3DS__
        glTexImage2D(
            GL_TEXTURE_2D, 0, arg.internalFormat, w, h, 0, GL_RGBA,
            GL_UNSIGNED_BYTE, buf
        );
#endif
#endif
        free(buf);
#endif
#ifdef GEKKO
        tc->buf = texture(&arg, src, w, h);
        GX_InitTexObj(
            &tc->obj, tc->buf, w, h, arg.fmt,
            gdp_texture_cm_table[cms],
            gdp_texture_cm_table[cmt],
            GX_FALSE
        );
        GX_InitTexObjFilterMode(&tc->obj, gdp_tf, gdp_tf);
        GX_InvalidateTexAll();
        GX_LoadTexObj(&tc->obj, GX_TEXMAP0);
#endif
    }
    else
    {
#ifdef GSP_DEBUG
        wdebug(
            "unknown texture fmt G_IM_FMT_%s, G_IM_SIZ_%db\n",
            str_im_fmt[fmt >> 2], 4 << (fmt & 3)
        );
#endif
    }
}

static void gsp_flush_texture(void)
{
    TILE *tile = &gdp_tile[0];
    void *src = &gdp_tmem[tile->tmem << 3];
    uint w = gdp_texture_size[0];
    uint h = gdp_texture_size[1];
    if (w == 0)
    {
        if (tile->line > 0)
        {
            uint x = tile->line << 4;
            w = x >> (tile->fmt & 3);
            h = h / x;
        }
        else
        {
            w = tile->lr[0] - tile->ul[0];
            h = tile->lr[1] - tile->ul[1];
        }
    }
    gdp_texture_scale[0] = gdp_texture_shift_table[tile->shift[0]] / w;
    gdp_texture_scale[1] = gdp_texture_shift_table[tile->shift[1]] / h;
    memcpy(gdp_tmem+0x1000, gdp_tmem, 0x1000);
    gdp_set_texture(
        gdp_timg, src, tile->fmt, gdp_tf, w, h,
        tile->cm[0], tile->cm[1]
    );
}

static void gsp_flush_fog(void)
{
#ifndef GSP_SWFOG
    if (gsp_geometry_mode & G_FOG)
    {
#if defined(__NATIVE__) || defined(__3DS__)
        GLfloat col[4];
        col[0] = (1.0F/0xFF) * gdp_fog[0];
        col[1] = (1.0F/0xFF) * gdp_fog[1];
        col[2] = (1.0F/0xFF) * gdp_fog[2];
        col[3] = (1.0F/0xFF) * gdp_fog[3];
        glEnable(GL_FOG);
        glFogfv(GL_FOG_COLOR, col);
#endif
#ifdef GEKKO
        float start = (float)(0x000-gsp_fog_o)/gsp_fog_m;
        float end   = (float)(0x100-gsp_fog_o)/gsp_fog_m;
        GXColor col = {gdp_fog[0], gdp_fog[1], gdp_fog[2], gdp_fog[3]};
        GX_SetFog(GX_FOG_LIN, 1+start, 1+end, 1, 2, col);
#endif
#ifdef __NDS__
        glEnable(GL_FOG);
        glFogColor(
            gdp_fog[0] >> 3,
            gdp_fog[1] >> 3,
            gdp_fog[2] >> 3,
            gdp_fog[3] >> 3
        );
        gsp_polyfmt |= POLY_FOG;
#endif
    }
    else
    {
#if defined(__NATIVE__) || defined(__3DS__)
        glDisable(GL_FOG);
#endif
#ifdef GEKKO
        GX_SetFog(GX_FOG_NONE, 0, 0, 0, 0, (GXColor){0});
#endif
#ifdef __NDS__
        glDisable(GL_FOG);
        gsp_polyfmt &= ~POLY_FOG;
#endif
    }
#endif
}

#define gsp_flush_mp()          gsp_set_mp(MP)
#ifdef GSP_SWVTX
#define gsp_flush_mm()
#else
#define gsp_flush_mm()          gsp_set_mm(MM)
#endif
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

#include "gdp/set_cc.c"

#define ZR  ((mode & Z_CMP) != 0)
#define ZW  ((mode & Z_UPD) != 0)
#define AC  ((mode & CVG_X_ALPHA) || (mode & 3) == G_AC_THRESHOLD)
#define BL  ((mode & 0x300) != CVG_DST_CLAMP && (mode & FORCE_BL))
#define DE  ((mode & 0xC00) == ZMODE_DEC)

#if defined(__NATIVE__) || defined(__3DS__)
#include "gdp/set_tx.gl.c"
#include "gdp/set_rm.gl.c"
#include "gdp/set_sc.gl.c"
#include "gsp/set_vp.gl.c"
#include "gsp/set_mtx.gl.c"
#include "gsp/set_cull.gl.c"
#endif
#ifdef GEKKO
#include "gdp/set_tx.gekko.c"
#include "gdp/set_rm.gekko.c"
#include "gdp/set_sc.gekko.c"
#include "gsp/set_vp.gekko.c"
#include "gsp/set_mtx.gekko.c"
#include "gsp/set_cull.gekko.c"
#endif
#ifdef __NDS__
#include "gdp/set_tx.nds.c"
#include "gdp/set_rm.nds.c"
#include "gdp/set_sc.nds.c"
#include "gsp/set_vp.nds.c"
#include "gsp/set_mtx.nds.c"
#include "gsp/set_cull.nds.c"
#endif

#undef ZR
#undef ZW
#undef AC
#undef BL
#undef DE

static void gdp_tri(const u8 *t)
{
    int i;
    if (gsp_new_texture)    {gsp_new_texture = FALSE; gsp_flush_texture();}
    if (gsp_new_fog)        {gsp_new_fog     = FALSE; gsp_flush_fog();    }
#ifdef __NDS__
    glPolyFmt(gsp_polyfmt);
#endif
#if defined(__NATIVE__) || defined(__NDS__) || defined(__3DS__)
    glBegin(GL_TRIANGLES);
#endif
#ifdef GEKKO
    GX_Begin(GX_TRIANGLES, GX_VTXFMT0, 3);
#endif
    for (i = 0; i < 3; i++)
    {
#ifndef GSP_SWVTX
        VTX  *v  = &gsp_vtx_buf[t[i]];
#endif
        VTXF *vf = &gsp_vtxf_buf[t[i]];
        float s = gdp_texture_scale[0] * vf->s;
        float t = gdp_texture_scale[1] * vf->t;
        u8 col[4];
        gdp_combine_cc(col, vf);
        gdp_combine_ac(col, vf);
#if defined(__NATIVE__) || defined(__3DS__)
        glColor4ub(col[0], col[1], col[2], col[3]);
        glTexCoord2f(s, t);
#ifndef GSP_SWFOG
        glFogCoordf(vf->shade[3]);
#endif
#ifdef GSP_SWVTX
        glVertex3f(vf->x, vf->y, vf->z);
#else
        glVertex3s(v->x, v->y, v->z);
#endif
#endif
#ifdef GEKKO
#ifdef GSP_SWVTX
        GX_Position3f32(vf->x, vf->y, vf->z);
#else
        GX_Position3s16(v->x, v->y, v->z);
#endif
        GX_Color4u8(col[0], col[1], col[2], col[3]);
        GX_TexCoord2f32(s, t);
#endif
#ifdef __NDS__
        glColor3b(col[0], col[1], col[2]);
        glTexCoord2f(s, t);
#ifdef GSP_SWVTX
        glVertex3v16(vf->x, vf->y, vf->z);
#else
        glVertex3v16(v->x, v->y, v->z);
#endif
#endif
    }
#if defined(__NATIVE__) || defined(__NDS__) || defined(__3DS__)
    glEnd();
#endif
#ifdef GEKKO
    GX_End();
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

#ifndef __NDS__
static void gsp_start_rect(void)
{
    if (gdp_rect == 0)
    {
#ifndef GSP_SWVTX
        f32 mf[4][4];
        mtx_identity(mf);
        gsp_set_mm(mf);
#endif
        gsp_set_vp(0, 1280, 0, 960);
        gsp_set_cull(G_CULL_BACK);
    }
}

static void gsp_start_fillrect(void)
{
    gsp_start_rect();
    if (gdp_rect != 1)
    {
        f32 mf[4][4];
        gdp_rect = 1;
        if (gdp_cycle) gdp_set_cc(0xFCFFFFFF, 0xFFFE793C);
        mtx_ortho(mf, 0, 1280, 960, 0, 0, 2);
        gsp_set_mp(mf);
    }
}

static void gsp_start_texrect(void)
{
    gsp_start_rect();
    if (gdp_rect != 2)
    {
        f32 mf[4][4];
        gdp_rect = 2;
        if (gdp_cycle) gdp_set_cc(0xFC121824, 0xFF33FFFF);
        mtx_ortho(
            mf,
#ifdef VIDEO_DYNRES
            4*video_l, 4*video_r,
#else
            0, 1280,
#endif
            960, 0, 0, 2
        );
        gsp_set_mp(mf);
    }
}
#endif

static void gsp_flush_rect(void)
{
#ifndef __NDS__
    if (gdp_rect != 0)
    {
        gdp_rect = 0;
        gdp_flush_cc();
        gsp_flush_mp();
        gsp_flush_mm();
        gsp_flush_vp();
        gsp_flush_cull();
    }
#endif
}

static void gsp_fillrect(u32 w0, u32 w1)
{
#ifndef __NDS__
    int i;
#ifndef GSP_SWVTX
    VTX  *v  = &gsp_vtx_buf[GSP_VTX_LEN];
#endif
    VTXF *vf = &gsp_vtxf_buf[GSP_VTX_LEN];
    int xh = w0 >> 12 & 0xFFF;
    int yh = w0 >>  0 & 0xFFF;
    int xl = w1 >> 12 & 0xFFF;
    int yl = w1 >>  0 & 0xFFF;
    uint r = RGBA16_R(gdp_fill);
    uint g = RGBA16_G(gdp_fill);
    uint b = RGBA16_B(gdp_fill);
    gsp_start_fillrect();
    if (gdp_cycle)
    {
        xh = (xh & ~3) + 4;
        yh = (yh & ~3) + 4;
    }
    for (i = 0; i < 4; i++)
    {
#ifdef GSP_SWVTX
        vf[i].x = i == 0 || i == 3 ? xl : xh;
        vf[i].y = i == 2 || i == 3 ? yl : yh;
        vf[i].z = 0;
#else
        v[i].x = i == 0 || i == 3 ? xl : xh;
        v[i].y = i == 2 || i == 3 ? yl : yh;
        v[i].z = 0;
#endif
        vf[i].shade[0] = r;
        vf[i].shade[1] = g;
        vf[i].shade[2] = b;
        vf[i].shade[3] = 0xFF;
    }
    gdp_tri(gdp_rect_tri[0]);
    gdp_tri(gdp_rect_tri[1]);
#else
    (void)w0;
    (void)w1;
#endif
}

static void gsp_texrect(void)
{
#ifndef __NDS__
    int i;
#ifndef GSP_SWVTX
    VTX  *v  = &gsp_vtx_buf[GSP_VTX_LEN];
#endif
    VTXF *vf = &gsp_vtxf_buf[GSP_VTX_LEN];
    int   xh = (s16)(gdp_texrect[0] >> 8) >> 4;
    int   yh = (s16)(gdp_texrect[0] << 4) >> 4;
    int   xl = (s16)(gdp_texrect[1] >> 8) >> 4;
    int   yl = (s16)(gdp_texrect[1] << 4) >> 4;
    float sl = (s16)(gdp_texrect[2] >> 16);
    float tl = (s16)(gdp_texrect[2] >>  0);
    float sh;
    float th;
    float dsdx = (32/4.0F/0x0400) * (s16)(gdp_texrect[3] >> 16);
    float dtdy = (32/4.0F/0x0400) * (s16)(gdp_texrect[3] >>  0);
    gsp_start_texrect();
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
            sl += 32*0.5F;
            tl += 32*0.5F;
        }
    }
    sh = sl + dsdx*(xh-xl);
    th = tl + dtdy*(yh-yl);
    if (gdp_texrect[0] >> 24 & 1)
    {
        vf[0].s = sh;
        vf[0].t = tl;
        vf[2].s = sl;
        vf[2].t = th;
    }
    else
    {
        vf[0].s = sl;
        vf[0].t = th;
        vf[2].s = sh;
        vf[2].t = tl;
    }
    vf[1].s = sh;
    vf[1].t = th;
    vf[3].s = sl;
    vf[3].t = tl;
    for (i = 0; i < 4; i++)
    {
#ifdef GSP_SWVTX
        vf[i].x = i == 0 || i == 3 ? xl : xh;
        vf[i].y = i == 2 || i == 3 ? yl : yh;
        vf[i].z = 0;
#else
        v[i].x = i == 0 || i == 3 ? xl : xh;
        v[i].y = i == 2 || i == 3 ? yl : yh;
        v[i].z = 0;
#endif
        vf[i].shade[0] = 0xFF;
        vf[i].shade[1] = 0xFF;
        vf[i].shade[2] = 0xFF;
        vf[i].shade[3] = 0xFF;
    }
    gdp_tri(gdp_rect_tri[0]);
    gdp_tri(gdp_rect_tri[1]);
#endif
}

#ifdef GSP_F3DEX2
static void gsp_bg(uObjBg *bg)
{
    int i;
#ifndef GSP_SWVTX
    VTX  *v  = &gsp_vtx_buf[GSP_VTX_LEN];
#endif
    VTXF *vf = &gsp_vtxf_buf[GSP_VTX_LEN];
    TILE *tile = &gdp_tile[0];
    void *timg = gsp_addr(bg->image_ptr);
    void *src;
    int  xl = bg->frame_x;
    int  yl = bg->frame_y;
    int  xh = xl + bg->image_w*bg->scale_w/0x400;
    int  yh = yl + bg->image_h*bg->scale_h/0x400;
    uint w = bg->image_w/4;
    uint h = bg->image_h/4;
    uint size = w*h << bg->image_siz >> 1;
    gdp_texture_scale[0] = 1;
    gdp_texture_scale[1] = 1;
    tile->pal = bg->image_pal;
    src = malloc(size);
    byteswap(src, timg, size);
    gdp_set_texture(
        timg, src, bg->image_fmt << 2 | bg->image_siz, gdp_tf, w, h,
        G_TX_WRAP, G_TX_WRAP
    );
    free(src);
    gdp_set_sc(bg->frame_x, bg->frame_y, bg->frame_w, bg->frame_h);
    gsp_start_texrect();
    for (i = 0; i < 4; i++)
    {
#ifdef GSP_SWVTX
        vf[i].x  = i == 0 || i == 3 ? xl : xh;
        vf[i].y  = i == 2 || i == 3 ? yl : yh;
        vf[i].z  = 0;
#else
        v[i].x  = i == 0 || i == 3 ? xl : xh;
        v[i].y  = i == 2 || i == 3 ? yl : yh;
        v[i].z  = 0;
#endif
        vf[i].s = i == 0 || i == 3 ?  0 :  1;
        vf[i].t = i == 2 || i == 3 ?  0 :  1;
        vf[i].shade[0] = 0xFF;
        vf[i].shade[1] = 0xFF;
        vf[i].shade[2] = 0xFF;
        vf[i].shade[3] = 0xFF;
    }
    gdp_tri(gdp_rect_tri[0]);
    gdp_tri(gdp_rect_tri[1]);
}
#endif

#ifdef GSP_F3DEX2
static void gsp_start_F3DEX2(void);
static void gsp_start_S2DEX2(void);
#endif
static void gsp_start(PTR ucode, u32 *dl)
{
    switch (ucode)
    {
#ifdef GSP_F3DEX
#ifdef APP_UNKT
#ifdef APP_E00
        /* F3DEX */
        case 0x000D9040:    break;
        /* F3DLX */
        case 0x000DA420:    break;
#endif
#endif
#ifdef APP_UCZL
#if defined(APP_J00) || defined(APP_E00)
        case 0x000E3F70:    gsp_start_F3DEX2(); break;
        case 0x000E5300:    gsp_start_S2DEX2(); break;
#endif
#endif
#ifdef APP_UNK4
#ifdef APP_E00
        case 0x00039E90:    gsp_start_F3DEX2(); break;
        case 0x0003B220:
            pdebug("notice: using L3DEX2\n");
            gsp_start_F3DEX2();
            break;
        case 0x0003C3B0:    gsp_start_S2DEX2(); break;
#endif
#endif
        default:
            edebug("unknown ucode 0x%08" FMT_X "\n", ucode);
            break;
#endif
    }
    gdp_combine_cc = gdp_combine_cc_0;
    gdp_combine_ac = gdp_combine_ac_0;
    gdp_triangle = gdp_tri;
    gdp_rect = 0;
    gsp_light_buf[0].x =   0;
    gsp_light_buf[0].y = 127;
    gsp_light_buf[0].z =   0;
    gsp_light_buf[1].x = 127;
    gsp_light_buf[1].y =   0;
    gsp_light_buf[1].z =   0;
    gsp_mtx_modelview = gsp_mtx_modelview_stack;
    gsp_dl_stack[0] = dl;
    gsp_dl_index = 0;
    gsp_new_fog   = FALSE;
    gsp_new_light = TRUE;
}

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
#define g_culldl                NULL
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

#define g_quad                  g_tri2

static void g_special_3(UNUSED u32 w0, UNUSED u32 w1)
{
    edebug("G_SPECIAL_3\n");
}

static void g_special_2(UNUSED u32 w0, UNUSED u32 w1)
{
    edebug("G_SPECIAL_2\n");
}

#ifdef APP_UNK4
#include "gsp/g_special_1.c"
#else
static void g_special_1(UNUSED u32 w0, UNUSED u32 w1)
{
    edebug("G_SPECIAL_1\n");
}
#endif

static void g_dma_io(UNUSED u32 w0, UNUSED u32 w1)
{
    edebug("G_DMA_IO\n");
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

static void g_obj_rectangle(UNUSED u32 w0, UNUSED u32 w1)
{
    edebug("G_OBJ_RECTANGLE\n");
}

static void g_obj_sprite(UNUSED u32 w0, UNUSED u32 w1)
{
    edebug("G_OBJ_SPRITE\n");
}

static void g_select_dl(UNUSED u32 w0, UNUSED u32 w1)
{
    edebug("G_SELECT_DL\n");
}

#include "gsp/g_obj_loadtxtr.c"
#include "gsp/g_bg_1cyc.c"
#include "gsp/g_bg_copy.c"
#include "gsp/g_obj_rendermode.c"

static void g_obj_rectangle_r(UNUSED u32 w0, UNUSED u32 w1)
{
    edebug("G_OBJ_RECTANGLE_R\n");
}

static void g_obj_movemem(UNUSED u32 w0, UNUSED u32 w1)
{
    edebug("G_OBJ_MOVEMEM\n");
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

static GSPCALL *gsp_table[] =
{
    /* 0x00 */  g_spnoop,
#ifdef GSP_F3D
    /* 0x01 */  g_mtx,
    /* 0x02 */  NULL,
    /* 0x03 */  g_movemem,
    /* 0x04 */  g_vtx,
    /* 0x05 */  NULL,
    /* 0x06 */  g_dl,
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
    /* 0xAF */  g_load_ucode,
    /* 0xB0 */  g_branch_z,
    /* 0xB1 */  g_tri2,
    /* 0xB2 */  g_modifyvtx,
    /* 0xB3 */  g_rdphalf_2,
    /* 0xB4 */  g_rdphalf_1,
    /* 0xB5 */  g_quad,
#else
    /* 0xAF */  NULL,
    /* 0xB0 */  NULL,
#ifdef GSP_F3D_20D
    /* 0xB1 */  g_rdphalf_cont,
    /* 0xB2 */  g_rdphalf_2,
    /* 0xB3 */  g_rdphalf_1,
    /* 0xB4 */  g_perspnormalize,
#else
    /* 0xB1 */  NULL,
    /* 0xB2 */  g_rdphalf_cont,
    /* 0xB3 */  g_rdphalf_2,
    /* 0xB4 */  g_rdphalf_1,
#endif
    /* 0xB5 */  NULL,
#endif
    /* 0xB6 */  g_cleargeometrymode,
    /* 0xB7 */  g_setgeometrymode,
    /* 0xB8 */  g_enddl,
    /* 0xB9 */  g_setothermode_l,
    /* 0xBA */  g_setothermode_h,
    /* 0xBB */  g_texture,
    /* 0xBC */  g_moveword,
    /* 0xBD */  g_popmtx,
    /* 0xBE */  g_culldl,
    /* 0xBF */  g_tri1,
    /* 0xC0 */  g_noop,
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
    /* 0xD3 */  g_special_3,
    /* 0xD4 */  g_special_2,
    /* 0xD5 */  g_special_1,
    /* 0xD6 */  g_dma_io,
    /* 0xD7 */  g_texture,
    /* 0xD8 */  g_popmtx,
    /* 0xD9 */  g_geometrymode,
    /* 0xDA */  g_mtx,
    /* 0xDB */  g_moveword,
    /* 0xDC */  g_movemem,
    /* 0xDD */  g_load_ucode,
    /* 0xDE */  g_dl,
    /* 0xDF */  g_enddl,
    /* 0xE0 */  g_noop,
    /* 0xE1 */  g_rdphalf_1,
    /* 0xE2 */  g_setothermode_l,
    /* 0xE3 */  g_setothermode_h,
#endif
    /* 0xE4 */  g_texrect,
    /* 0xE5 */  g_texrectflip,
    /* 0xE6 */  g_rdploadsync,
    /* 0xE7 */  g_rdppipesync,
    /* 0xE8 */  g_rdptilesync,
    /* 0xE9 */  g_rdpfullsync,
    /* 0xEA */  g_setkeygb,
    /* 0xEB */  g_setkeyr,
    /* 0xEC */  g_setconvert,
    /* 0xED */  g_setscissor,
    /* 0xEE */  g_setprimdepth,
    /* 0xEF */  g_rdpsetothermode,
    /* 0xF0 */  g_loadtlut,
#ifdef GSP_F3DEX2
    /* 0xF1 */  g_rdphalf_2,
#else
    /* 0xF1 */  NULL,
#endif
    /* 0xF2 */  g_settilesize,
    /* 0xF3 */  g_loadblock,
    /* 0xF4 */  g_loadtile,
    /* 0xF5 */  g_settile,
    /* 0xF6 */  g_fillrect,
    /* 0xF7 */  g_setfillcolor,
    /* 0xF8 */  g_setfogcolor,
    /* 0xF9 */  g_setblendcolor,
    /* 0xFA */  g_setprimcolor,
    /* 0xFB */  g_setenvcolor,
    /* 0xFC */  g_setcombine,
    /* 0xFD */  g_settimg,
    /* 0xFE */  g_setzimg,
    /* 0xFF */  g_setcimg,
};

#ifdef GSP_F3DEX2
static GSPCALL *const gsp_table_F3DEX2[] =
{
    /* 0x01 */  g_vtx,
    /* 0x02 */  g_modifyvtx,
    /* 0x03 */  g_culldl,
    /* 0x04 */  g_branch_z,
    /* 0x05 */  g_tri1,
    /* 0x06 */  g_tri2,
    /* 0x07 */  g_quad,
    /* 0x08 */  NULL,
    /* 0x09 */  NULL,
    /* 0x0A */  NULL,
    /* 0x0B */  NULL,
};

static GSPCALL *const gsp_table_S2DEX2[] =
{
    /* 0x01 */  g_obj_rectangle,
    /* 0x02 */  g_obj_sprite,
    /* 0x03 */  g_culldl,
    /* 0x04 */  g_select_dl,
    /* 0x05 */  g_obj_loadtxtr,
    /* 0x06 */  g_obj_loadtxtr,
    /* 0x07 */  g_obj_loadtxtr,
    /* 0x08 */  g_obj_loadtxtr,
    /* 0x09 */  g_bg_1cyc,
    /* 0x0A */  g_bg_copy,
    /* 0x0B */  g_obj_rendermode,
};

static void gsp_start_F3DEX2(void)
{
    memcpy(&gsp_table[0x01], gsp_table_F3DEX2, sizeof(gsp_table_F3DEX2));
    gsp_table[G_MTX]     = g_mtx;
    gsp_table[G_MOVEMEM] = g_movemem;
}

static void gsp_start_S2DEX2(void)
{
    memcpy(&gsp_table[0x01], gsp_table_S2DEX2, sizeof(gsp_table_S2DEX2));
    gsp_table[G_OBJ_RECTANGLE_R] = g_obj_rectangle_r;
    gsp_table[G_OBJ_MOVEMEM]     = g_obj_movemem;
}
#endif

static void gsp_write(void)
{
#ifdef __NATIVE__
    /*
    u16 *buf = malloc(4*video_w*video_h);
    uint y;
    uint x;
    glReadPixels(
        0, 0, video_w, video_h, GL_RGBA, GL_UNSIGNED_SHORT_5_5_5_1, buf
    );
    for (y = 0; y < 240; y++)
    {
        for (x = 0; x < 320; x++)
        {
            uint dx = (    x) * video_w/320;
            uint dy = (240-y) * video_h/240;
            gdp_cimg[(320*y+x)^(AX_B>>1)] = buf[video_w*dy+dx];
        }
    }
    free(buf);
    */
#endif
}

#ifdef GSP_DEBUG
static const u8 gsp_cmd[] =
{
#ifdef GSP_F3DEX2
    0x00, 0x04, 0xB2, 0xBE, 0xB0, 0xBF, 0xB1, 0xB5,
    0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
    0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
    0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
    0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
    0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F,
    0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57,
    0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F,
    0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67,
    0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F,
    0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77,
    0x78, 0x79, 0x7A, 0x7B, 0x7C, 0x7D, 0x7E, 0x7F,
    0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87,
    0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E, 0x8F,
    0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97,
    0x98, 0x99, 0x9A, 0x9B, 0x9C, 0x9D, 0x9E, 0x9F,
    0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7,
    0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF,
    0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7,
    0xB8, 0xB9, 0xBA, 0xBB, 0xBC, 0xBD, 0xBE, 0xBF,
    0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7,
    0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF,
    0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xBB,
    0xBD, 0xB7, 0x01, 0xBC, 0x03, 0xAF, 0x06, 0xB8,
    0xC0, 0xB3, 0xB9, 0xBA, 0xE4, 0xE5, 0xE6, 0xE7,
    0xE8, 0xE9, 0xEA, 0xEB, 0xEC, 0xED, 0xEE, 0xEF,
    0xF0, 0xB4, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7,
    0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF,
#else
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
    0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
    0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
    0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
    0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F,
    0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57,
    0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F,
    0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67,
    0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F,
    0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77,
    0x78, 0x79, 0x7A, 0x7B, 0x7C, 0x7D, 0x7E, 0x7F,
    0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87,
    0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E, 0x8F,
    0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97,
    0x98, 0x99, 0x9A, 0x9B, 0x9C, 0x9D, 0x9E, 0x9F,
    0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7,
    0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF,
    0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7,
    0xB8, 0xB9, 0xBA, 0xBB, 0xBC, 0xBD, 0xBE, 0xBF,
    0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7,
    0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF,
    0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7,
    0xD8, 0xD9, 0xDA, 0xDB, 0xDC, 0xDD, 0xDE, 0xDF,
    0xE0, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7,
    0xE8, 0xE9, 0xEA, 0xEB, 0xEC, 0xED, 0xEE, 0xEF,
    0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7,
    0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF,
#endif
};
#endif

void gsp_init(void)
{
#ifdef __3DS__
    pglInit();
#endif
#if defined(__NATIVE__) || defined(__3DS__)
    glEnable(GL_SCISSOR_TEST);
    glEnable(GL_POLYGON_OFFSET_FILL);
#ifdef __3DS__
    glDepthFunc(GL_LESS);
#endif
    glAlphaFunc(GL_GEQUAL, 1.0F/8);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
#ifndef GSP_SWFOG
    glFogi(GL_FOG_MODE, GL_LINEAR);
    glFogf(GL_FOG_START, 0x00);
    glFogf(GL_FOG_END,   0xFF);
    glFogi(GL_FOG_COORD_SRC, GL_FOG_COORD);
#endif
#endif
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
#endif
#ifdef __NDS__
    glInit();
    glAlphaFunc(2);
#endif
    gdp_txtcache = NULL;
    gsp_new_cache = FALSE;
}

void gsp_exit(void)
{
#ifdef __3DS__
    pglExit();
#endif
}

void gsp_cache(void)
{
    gsp_new_cache = TRUE;
}

void gsp_update(PTR ucode, u32 *dl)
{
    int i;
#ifdef GSP_SWVTX
    f32 mf[4][4];
#endif
#ifdef APP_UNK4
    gsp_new_cache = TRUE;
#endif
    if (gsp_new_cache)
    {
        TXTCACHE *tc;
        TXTCACHE *tcn;
        gsp_new_cache = FALSE;
        for (tc = gdp_txtcache; tc != NULL; tc = tcn)
        {
            tcn = tc->next;
#if defined(__NATIVE__) || defined(__NDS__) || defined(__3DS__)
            glDeleteTextures(1, &tc->name);
#endif
#ifdef GEKKO
            free(tc->buf);
#endif
            free(tc);
        }
        gdp_txtcache = NULL;
    }
#if defined(__NATIVE__) || defined(__3DS__)
    glViewport(0, 0, video_w, video_h);
    glScissor(0, 0, video_w, video_h);
    glDepthMask(GL_TRUE);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
#endif
#ifdef GEKKO
    GX_SetViewport(0, 0, video_w, video_h, 0, 1);
    GX_SetScissor(0, 0, video_w, video_h);
    GX_InvVtxCache();
    gsp_decal = FALSE;
#endif
#ifdef __NDS__
    glViewport(0, 0, video_w-1, video_h-1);
    gsp_polyfmt =
        POLY_ID(0) | POLY_ALPHA(31) | POLY_CULL_NONE | POLY_MODULATION;
#endif
#ifdef __3DS__
    pglSelectScreen(GFX_TOP, GFX_LEFT);
#endif
#ifdef GSP_SWVTX
    mtx_identity(mf);
    gsp_set_mm(mf);
#endif
    for (i = 0; i < 16; i++) gsp_addr_table[i] = cpu_dram;
    gsp_new_texture = FALSE;
    gsp_start(ucode, dl);
    do
    {
        void (*cmd)(u32, u32);
        u32 w0 = gsp_dl_stack[gsp_dl_index][0];
        u32 w1 = gsp_dl_stack[gsp_dl_index][1];
#ifdef GSP_DEBUG
        pdebug(
            "0x%08X: (%02X) %08X %08X\n",
            __ptr(gsp_dl_stack[gsp_dl_index]), gsp_cmd[w0 >> 24], w0, w1
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
            wdebug("invalid Gfx {{0x%08" FMT_X ", 0x%08" FMT_X "}}\n", w0, w1);
        }
    }
    while (gsp_dl_index >= 0);
    gsp_write();
#ifdef __NATIVE__
    SDL_GL_SwapWindow(window);
#endif
#ifdef GEKKO
    GX_DrawDone();
    GX_SetZMode(GX_TRUE, GX_ALWAYS, GX_TRUE);
    GX_SetColorUpdate(GX_TRUE);
    GX_CopyDisp(framebuffer, GX_TRUE);
    GX_Flush();
#endif
#ifdef __NDS__
    glFlush(0);
#endif
#ifdef __3DS__
    pglSwapBuffers();
#endif
}

void gsp_image(UNUSED void *img)
{
#ifdef __NATIVE__
    /*
    void *buf;
    GLuint name;
    buf = malloc(2*320*240);
    wordswap(buf, img, 2*320*240);
    glViewport(0, 0, video_w, video_h);
    glScissor(0, 0, video_w, video_h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    glDisable(GL_ALPHA_TEST);
    glDisable(GL_BLEND);
    glPolygonOffset(0, 0);
    glDisable(GL_FOG);
    glDisable(GL_CULL_FACE);
    glEnable(GL_TEXTURE_2D);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glGenTextures(1, &name);
    glBindTexture(GL_TEXTURE_2D, name);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(
        GL_TEXTURE_2D, 0, GL_RGBA, 320, 240, 0, GL_RGBA,
        GL_UNSIGNED_SHORT_5_5_5_1, buf
    );
    glBegin(GL_QUADS);
    glTexCoord2f(0, 1); glVertex3s(-1, -1, 0);
    glTexCoord2f(1, 1); glVertex3s( 1, -1, 0);
    glTexCoord2f(1, 0); glVertex3s( 1,  1, 0);
    glTexCoord2f(0, 0); glVertex3s(-1,  1, 0);
    glEnd();
    glDeleteTextures(1, &name);
    SDL_GL_SwapWindow(window);
    free(buf);
    */
#endif
}

#endif
