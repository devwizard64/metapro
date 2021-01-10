#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef GEKKO
#include <malloc.h>
#endif

#include <math.h>
#ifndef GEKKO
#include <GL/gl.h>
#endif
#ifdef _3DS
#include <GL/picaGL.h>
#endif

#ifdef GSP_LEGACY
#ifndef _NATIVE
#define glVertex3s(x, y, z) glVertex3f(x, y, z)
#endif
#endif

#ifndef GL_CLAMP_TO_EDGE
#define GL_CLAMP_TO_EDGE 0x812F
#endif
#ifndef GL_MIRRORED_REPEAT
#define GL_MIRRORED_REPEAT 0x8370
#endif

#include "types.h"
#include "cpu.h"
#include "lib.h"
#include "app.h"

#ifndef APP_SEQ

#include "gbi.h"

#define CHANGE_MTXF_PROJECTION  0x01
#define CHANGE_MTXF_MODELVIEW   0x02
#define CHANGE_VIEWPORT         0x04
#define CHANGE_CULL             0x08
#define CHANGE_RENDERMODE       0x10
#define CHANGE_SCISSOR          0x20
#define CHANGE_TEXTURE_ENABLED  0x40
#define CHANGE_TEXTURE          0x80

#ifdef APP_UNSM
#define GSP_OUTPUT_LEN  (3*0x1800)
#define GSP_TEXTURE_LEN 0x100
#endif
#ifdef APP_UNK4
#define GSP_OUTPUT_LEN  (3*0x4000)
#define GSP_TEXTURE_LEN 0x200
#endif

#ifdef GSP_F3D
#define GSP_VTX_LEN     0x10
#endif
#ifdef GSP_F3DEX2
#define GSP_VTX_LEN     0x20
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
#define IA8_I(x)        (((x) << 0 & 0xF0) | ((x) >> 4       ))
#define IA8_A(x)        (((x) << 4       ) | ((x) >> 0 & 0x0F))
#define I4_IH           IA8_I
#define I4_IL           IA8_A

#define CC(a, b, c, d)                                                         \
(                                                                              \
    ((G_CCMUX_##a) << 12 & 0xF000U) |                                          \
    ((G_CCMUX_##b) <<  8 & 0x0F00U) |                                          \
    ((G_CCMUX_##c) <<  3 & 0x00F8U) |                                          \
    ((G_CCMUX_##d) <<  0 & 0x0007U)                                            \
)
#define AC(a, b, c, d)                                                         \
(                                                                              \
    ((G_ACMUX_##a) <<  9 & 0x0E00U) |                                          \
    ((G_ACMUX_##b) <<  6 & 0x01C0U) |                                          \
    ((G_ACMUX_##c) <<  3 & 0x0038U) |                                          \
    ((G_ACMUX_##d) <<  0 & 0x0007U)                                            \
)
#define CC1(a, b, c, d) ((u32)CC(a, b, c, d) << 16 | (u32)CC(a, b, c, d))
#define AC1(a, b, c, d) ((u32)AC(a, b, c, d) << 12 | (u32)AC(a, b, c, d))
#define CC2(a0, b0, c0, d0, a1, b1, c1, d1) \
    ((u32)CC(a0, b0, c0, d0) << 16 | (u32)CC(a1, b1, c1, d1))
#define AC2(a0, b0, c0, d0, a1, b1, c1, d1) \
    ((u32)AC(a0, b0, c0, d0) << 12 | (u32)AC(a1, b1, c1, d1))

#define MP (*gsp_mtxf_projection)
#define MM (*gsp_mtxf_modelview)

#ifndef GEKKO
struct texture_t
{
    void  *addr;
    GLuint filter;
};
#endif

struct viewport_t
{
#ifdef _EB
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

struct vtx_t
{
#ifdef _EB
    s16 x;
    s16 y;
    s16 z;
    u16 f;
    s16 u;
    s16 v;
    s8  r;
    s8  g;
    s8  b;
    u8  a;
#else
    s16 y;
    s16 x;
    u16 f;
    s16 z;
    s16 v;
    s16 u;
    u8  a;
    s8  b;
    s8  g;
    s8  r;
#endif
};

struct vtxf_t
{
    f32 u;
    f32 v;
    u8  shade[4];
};

struct light_t
{
    struct
    {
    #ifdef _EB
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
#ifdef _EB
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

struct lightf_t
{
    f32 r;
    f32 g;
    f32 b;
    f32 x;
    f32 y;
    f32 z;
};

struct obj_bg_t
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

struct obj_sprite_t
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

struct obj_mtx_t
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

struct obj_sub_mtx_t
{
    s16 y;
    s16 x;
    u16 base_scale_y;
    u16 base_scale_x;
};

struct obj_txtr_t
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

static void gsp_texture_read_rgba16(void *, const void *, uint);
#if 0
static void gsp_texture_read_rgba32(void *, const void *, uint);
static void gsp_texture_read_yuv16(void *, const void *, uint);
#endif
#ifndef APP_UNSM
#endif
static void gsp_texture_read_ia4(void *, const void *, uint);
static void gsp_texture_read_ia8(void *, const void *, uint);
static void gsp_texture_read_ia16(void *, const void *, uint);
#ifndef APP_UNSM
static void gsp_texture_read_i4(void *, const void *, uint);
static void gsp_texture_read_i8(void *, const void *, uint);
#endif

#if 0
static void gsp_g_spnoop(u32, u32);
#endif
#ifdef GSP_F3D
static void gsp_g_mtx(u32, u32);
static void gsp_g_movemem(u32, u32);
static void gsp_g_vtx(u32, u32);
static void gsp_g_dl(u32, u32);
#if 0
static void gsp_g_rdphalf_cont(u32, u32);
#endif
static void gsp_g_rdphalf_2(u32, u32);
static void gsp_g_rdphalf_1(u32, u32);
#ifdef GSP_F3D_20D
#ifdef _DEBUG
static void gsp_g_perspnorm(u32, u32);
#endif
#endif
static void gsp_g_cleargeometrymode(u32, u32);
static void gsp_g_setgeometrymode(u32, u32);
static void gsp_g_enddl(u32, u32);
static void gsp_g_setothermode_l(u32, u32);
static void gsp_g_setothermode_h(u32, u32);
static void gsp_g_texture(u32, u32);
static void gsp_g_moveword(u32, u32);
static void gsp_g_popmtx(u32, u32);
#if 0
static void gsp_g_culldl(u32, u32);
#endif
static void gsp_g_tri1(u32, u32);
#if 0
static void gsp_g_noop(u32, u32);
#endif
#endif
#ifdef GSP_F3DEX2
static void gsp_g_vtx(u32, u32);
static void gsp_g_modifyvtx(u32, u32);
static void gsp_g_culldl(u32, u32);
static void gsp_g_branch_z(u32, u32);
static void gsp_g_tri1(u32, u32);
static void gsp_g_tri2(u32, u32);
static void gsp_g_quad(u32, u32);
static void gsp_g_special_3(u32, u32);
static void gsp_g_special_2(u32, u32);
static void gsp_g_special_1(u32, u32);
static void gsp_g_dma_io(u32, u32);
static void gsp_g_texture(u32, u32);
static void gsp_g_popmtx(u32, u32);
static void gsp_g_geometrymode(u32, u32);
static void gsp_g_mtx(u32, u32);
static void gsp_g_moveword(u32, u32);
static void gsp_g_movemem(u32, u32);
static void gsp_g_load_ucode(u32, u32);
static void gsp_g_dl(u32, u32);
static void gsp_g_enddl(u32, u32);
#if 0
static void gsp_g_noop(u32, u32);
#endif
static void gsp_g_rdphalf_1(u32, u32);
static void gsp_g_setothermode_l(u32, u32);
static void gsp_g_setothermode_h(u32, u32);
#endif
static void gsp_g_texrect(u32, u32);
static void gsp_g_texrectflip(u32, u32);
#ifdef _DEBUG
static void gsp_g_rdploadsync(u32, u32);
static void gsp_g_rdppipesync(u32, u32);
static void gsp_g_rdptilesync(u32, u32);
static void gsp_g_rdpfullsync(u32, u32);
#endif
#if 0
static void gsp_g_setkeygb(u32, u32);
static void gsp_g_setkeyr(u32, u32);
static void gsp_g_setconvert(u32, u32);
#endif
static void gsp_g_setscissor(u32, u32);
#ifdef APP_UNK4
static void gsp_g_setprimdepth(u32, u32);
#endif
#if 0
static void gsp_g_rdpsetothermode(u32, u32);
#endif
#ifdef APP_UNK4
static void gsp_g_loadtlut(u32, u32);
static void gsp_g_rdphalf_2(u32, u32);
#endif
static void gsp_g_settilesize(u32, u32);
static void gsp_g_loadblock(u32, u32);
static void gsp_g_loadtile(u32, u32);
static void gsp_g_settile(u32, u32);
static void gsp_g_fillrect(u32, u32);
static void gsp_g_setfillcolor(u32, u32);
static void gsp_g_setfogcolor(u32, u32);
static void gsp_g_setblendcolor(u32, u32);
static void gsp_g_setprimcolor(u32, u32);
static void gsp_g_setenvcolor(u32, u32);
static void gsp_g_setcombine(u32, u32);
static void gsp_g_settimg(u32, u32);
#ifdef _DEBUG
static void gsp_g_setzimg(u32, u32);
static void gsp_g_setcimg(u32, u32);
#endif
#ifdef GSP_F3DEX2
static void gsp_g_obj_rectangle(u32, u32);
static void gsp_g_obj_sprite(u32, u32);
static void gsp_g_select_dl(u32, u32);
static void gsp_g_obj_loadtxtr(u32, u32);
static void gsp_g_obj_ldtx_sprite(u32, u32);
static void gsp_g_obj_ldtx_rect(u32, u32);
static void gsp_g_obj_ldtx_rect_r(u32, u32);
static void gsp_g_bg_1cyc(u32, u32);
static void gsp_g_bg_copy(u32, u32);
static void gsp_g_obj_rendermode(u32, u32);
static void gsp_g_obj_rectangle_r(u32, u32);
static void gsp_g_obj_movemem(u32, u32);
static void gsp_g_rdphalf_0(u32, u32);
#endif

#ifdef GEKKO
unused
#endif
static void (*gsp_texture_read_table[])(void *, const void *, uint) =
{
    /* RGBA  4 */ NULL,
    /* RGBA  8 */ NULL,
    /* RGBA 16 */ gsp_texture_read_rgba16,
#if 0
    /* RGBA 32 */ gsp_texture_read_rgba32,
#else
    /* RGBA 32 */ NULL,
#endif
    /* YUV   4 */ NULL,
    /* YUV   8 */ NULL,
#if 0
    /* YUV  16 */ gsp_texture_read_yuv16,
#else
    /* YUV  16 */ NULL,
#endif
    /* YUV  32 */ NULL,
    /* CI    4 */ NULL,
    /* CI    8 */ NULL,
    /* CI   16 */ NULL,
    /* CI   32 */ NULL,
    /* IA    4 */ gsp_texture_read_ia4,
    /* IA    8 */ gsp_texture_read_ia8,
    /* IA   16 */ gsp_texture_read_ia16,
    /* IA   32 */ NULL,
#ifdef APP_UNSM
    /* I     4 */ NULL,
    /* I     8 */ NULL,
#else
    /* I     4 */ gsp_texture_read_i4,
    /* I     8 */ gsp_texture_read_i8,
#endif
    /* I    16 */ NULL,
    /* I    32 */ NULL,
};

#ifdef GSP_F3DEX2
static void (*const gsp_table_3d[])(u32, u32) =
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

static void (*const gsp_table_2d[])(u32, u32) =
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

static void (*gsp_table[])(u32, u32) =
{
#if 0
    /* 0x00 */  gsp_g_spnoop,
#else
    /* 0x00 */  NULL,
#endif
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
    /* 0xAF */  NULL,
    /* 0xB0 */  NULL,
#ifdef GSP_F3D
#ifdef GSP_F3D_20D
#if 0
    /* 0xB1 */  gsp_g_rdphalf_cont,
#else
    /* 0xB1 */  NULL,
#endif
    /* 0xB2 */  gsp_g_rdphalf_2,
    /* 0xB3 */  gsp_g_rdphalf_1,
#ifdef _DEBUG
    /* 0xB4 */  gsp_g_perspnorm,
#else
    /* 0xB4 */  NULL,
#endif
#else
    /* 0xB1 */  NULL,
#if 0
    /* 0xB2 */  gsp_g_rdphalf_cont,
#else
    /* 0xB2 */  NULL,
#endif
    /* 0xB3 */  gsp_g_rdphalf_2,
    /* 0xB4 */  gsp_g_rdphalf_1,
#endif
    /* 0xB5 */  NULL,
    /* 0xB6 */  gsp_g_cleargeometrymode,
    /* 0xB7 */  gsp_g_setgeometrymode,
    /* 0xB8 */  gsp_g_enddl,
    /* 0xB9 */  gsp_g_setothermode_l,
    /* 0xBA */  gsp_g_setothermode_h,
    /* 0xBB */  gsp_g_texture,
    /* 0xBC */  gsp_g_moveword,
    /* 0xBD */  gsp_g_popmtx,
#if 0
    /* 0xBE */  gsp_g_culldl,
#else
    /* 0xBE */  NULL,
#endif
    /* 0xBF */  gsp_g_tri1,
#if 0
    /* 0xC0 */  gsp_g_noop,
#else
    /* 0xC0 */  NULL,
#endif
#endif
#ifdef GSP_F3DEX2
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
#if 0
    /* 0xE0 */  gsp_g_noop,
#else
    /* 0xE0 */  NULL,
#endif
    /* 0xE1 */  gsp_g_rdphalf_1,
    /* 0xE2 */  gsp_g_setothermode_l,
    /* 0xE3 */  gsp_g_setothermode_h,
#endif
    /* 0xE4 */  gsp_g_texrect,
    /* 0xE5 */  gsp_g_texrectflip,
#ifdef _DEBUG
    /* 0xE6 */  gsp_g_rdploadsync,
    /* 0xE7 */  gsp_g_rdppipesync,
    /* 0xE8 */  gsp_g_rdptilesync,
    /* 0xE9 */  gsp_g_rdpfullsync,
#else
    /* 0xE6 */  NULL,
    /* 0xE7 */  NULL,
    /* 0xE8 */  NULL,
    /* 0xE9 */  NULL,
#endif
#if 0
    /* 0xEA */  gsp_g_setkeygb,
    /* 0xEB */  gsp_g_setkeyr,
    /* 0xEC */  gsp_g_setconvert,
#else
    /* 0xEA */  NULL,
    /* 0xEB */  NULL,
    /* 0xEC */  NULL,
#endif
    /* 0xED */  gsp_g_setscissor,
#ifdef APP_UNK4
    /* 0xEE */  gsp_g_setprimdepth,
#else
    /* 0xEF */  NULL,
#endif
#if 0
    /* 0xEF */  gsp_g_rdpsetothermode,
#else
    /* 0xEE */  NULL,
#endif
#ifdef APP_UNSM
    /* 0xF0 */  NULL,
    /* 0xF1 */  NULL,
#endif
#ifdef APP_UNK4
    /* 0xF0 */  gsp_g_loadtlut,
    /* 0xF1 */  gsp_g_rdphalf_2,
#endif
    /* 0xF2 */  gsp_g_settilesize,
    /* 0xF3 */  gsp_g_loadblock,
    /* 0xF4 */  gsp_g_loadtile,
    /* 0xF5 */  gsp_g_settile,
    /* 0xF6 */  gsp_g_fillrect,
    /* 0xF7 */  gsp_g_setfillcolor,
    /* 0xF8 */  gsp_g_setfogcolor,
    /* 0xF9 */  gsp_g_setblendcolor,
    /* 0xFA */  gsp_g_setprimcolor,
    /* 0xFB */  gsp_g_setenvcolor,
    /* 0xFC */  gsp_g_setcombine,
    /* 0xFD */  gsp_g_settimg,
#ifdef _DEBUG
    /* 0xFE */  gsp_g_setzimg,
    /* 0xFF */  gsp_g_setcimg,
#else
    /* 0xFE */  NULL,
    /* 0xFF */  NULL,
#endif
};

static void (*gsp_combine_cc)(u8 *, struct vtxf_t *);
static void (*gsp_combine_ac)(u8 *, struct vtxf_t *);
#ifdef _3DS
static void (*gsp_write_triangle)(u8 *);
#else
#define gsp_write_triangle gsp_write_triangle_init
#endif
static void (*gsp_triangle)(u8 *);

#ifdef GEKKO
#else
static const GLuint gsp_texture_flag_table[] =
{
    /* 0x00 G_TX_NOMIRROR | G_TX_WRAP  */ GL_REPEAT,
    /* 0x01 G_TX_MIRROR   | G_TX_WRAP  */ GL_MIRRORED_REPEAT,
    /* 0x02 G_TX_NOMIRROR | G_TX_CLAMP */ GL_CLAMP_TO_EDGE,
    /* 0x03 G_TX_MIRROR   | G_TX_CLAMP */ GL_CLAMP_TO_EDGE,
};
#ifdef _3DS
static const GLint gsp_texture_fmt_table[] =
{
    /* RGBA  4 */ 0,
    /* RGBA  8 */ 0,
    /* RGBA 16 */ GPU_RGBA5551,
#if 0
    /* RGBA 32 */ GPU_RGBA8,
#else
    /* RGBA 32 */ 0,
#endif
    /* YUV   4 */ 0,
    /* YUV   8 */ 0,
    /* YUV  16 */ 0,
    /* YUV  32 */ 0,
    /* CI    4 */ 0,
    /* CI    8 */ 0,
    /* CI   16 */ 0,
    /* CI   32 */ 0,
    /* IA    4 */ GPU_LA4,
    /* IA    8 */ GPU_LA4,
    /* IA   16 */ GPU_LA8,
    /* IA   32 */ 0,
#if 0
    /* I     4 */ GPU_LA4,
    /* I     8 */ GPU_LA8,
#else
    /* I     4 */ 0,
    /* I     8 */ 0,
#endif
    /* I    16 */ 0,
    /* I    32 */ 0,
};
#endif
#endif

#ifndef GSP_LEGACY
static s16  gsp_output_vtx_buf[3*3*GSP_OUTPUT_LEN];
static f32  gsp_output_txc_buf[2*3*GSP_OUTPUT_LEN];
static u8   gsp_output_col_buf[4*3*GSP_OUTPUT_LEN];
static s16 *gsp_output_vtx;
static f32 *gsp_output_txc;
static u8  *gsp_output_col;
static s16 *gsp_output_v;
static f32 *gsp_output_t;
static u8  *gsp_output_c;
static u32  gsp_output_total;
static u32  gsp_output_count;
#endif

static struct viewport_t gsp_viewport;
static struct light_t    gsp_light_buf[10];
static struct lightf_t   gsp_lightf_buf[10];
static struct vtx_t      gsp_vtx_buf[GSP_VTX_LEN];
static struct vtxf_t     gsp_vtxf_buf[GSP_VTX_LEN];
#ifdef GSP_F3DEX2
static s16  gsp_mtx[32];
#endif
static f32  gsp_mtxf_projection_stack[2][4][4];
static f32  gsp_mtxf_modelview_stack[16][4][4];
#ifdef GSP_FOG
static f32  gsp_mtxf_mvp[4][4];
#endif
static f32  MP[4][4];
static f32  MM[4][4];
static u8  *gsp_addr_table[16];
static u32 *gsp_dl_stack[10];
static s32  gsp_dl_index;

#ifdef GSP_F3D
static void *const gsp_movemem_table[] =
{
    /* 0x80 G_MV_VIEWPORT */ &gsp_viewport,
    /* 0x82 G_MV_LOOKATY  */ &gsp_light_buf[1],
    /* 0x84 G_MV_LOOKATX  */ &gsp_light_buf[0],
    /* 0x86 G_MV_L0       */ &gsp_light_buf[2],
    /* 0x88 G_MV_L1       */ &gsp_light_buf[3],
    /* 0x8A G_MV_L2       */ &gsp_light_buf[4],
    /* 0x8C G_MV_L3       */ &gsp_light_buf[5],
    /* 0x8E G_MV_L4       */ &gsp_light_buf[6],
    /* 0x90 G_MV_L5       */ &gsp_light_buf[7],
    /* 0x92 G_MV_L6       */ &gsp_light_buf[8],
    /* 0x94 G_MV_L7       */ &gsp_light_buf[9],
    /* 0x96 G_MV_TXTATT   */ NULL,
};
#endif
#ifdef GSP_F3DEX2
static void *const gsp_movemem_table[] =
{
    /* 0x08 G_MV_VIEWPORT */ &gsp_viewport,
    /* 0x0A G_MV_LIGHT    */ gsp_light_buf,
    /* 0x0C G_MV_POINT    */ NULL,
    /* 0x0E G_MV_MATRIX   */ NULL,
};
#endif

static u32 gsp_light_no;
#ifdef GSP_FOG
static s16 gsp_fog_m;
static s16 gsp_fog_o;
#endif

static u32 gsp_geometry_mode;
static u32 gsp_othermode_l;
static u32 gsp_othermode_h;
static u32 gsp_cycle;

#ifdef GSP_F3DEX2
static u32 gsp_rdphalf_1;
#endif

static s16 gsp_scissor_l;
static s16 gsp_scissor_t;
static s16 gsp_scissor_r;
static s16 gsp_scissor_b;

static u16 gsp_fill;
static u8  gsp_fog[4];
#if 0
static u8  gsp_blend[4];
#endif
static u8  gsp_prim[4];
static u8  gsp_env[4];

static void *gsp_texture_buf;
static void *gsp_texture_addr;
static u8    gsp_texture_fmt;
static u16   gsp_texture_len;
static u8    gsp_texture_flag[2];
static u8    gsp_texture_shift[2];
static u16   gsp_texture_size[2];
static f32   gsp_texture_tscale[2];
static f32   gsp_texture_vscale[2];

static bool gsp_texrect_flip;
static s16  gsp_texrect_xh;
static s16  gsp_texrect_yh;
static s16  gsp_texrect_xl;
static s16  gsp_texrect_yl;
static f32  gsp_texrect_ul;
static f32  gsp_texrect_vl;
static f32  gsp_texrect_dsdx;
static f32  gsp_texrect_dtdy;

#ifdef GSP_F3DEX2
static u8  gsp_obj_rendermode;
#endif

#ifdef _3DS
static f32 gsp_depth;
#endif
static bool gsp_lookat;
static bool gsp_light_new;
static bool gsp_texture_enabled;
static u8  gsp_rect;
static u8  gsp_change;
static bool gsp_cache_flag;

#ifdef GEKKO
#define GL_NEAREST 0
#define GL_LINEAR  1
static u8     gsp_texture_filter;
#else
static struct texture_t gsp_texture_table[GSP_TEXTURE_LEN];
static GLuint gsp_texture_name_table[GSP_TEXTURE_LEN];
static u32    gsp_texture;
static GLuint gsp_texture_filter;
#endif

static void mtx_read(f32 *dst, const s16 *src)
{
    uint cnt = 4*4;
    do
    {
    #ifdef _EB
        dst[0x00] = (1.0F/0x10000) * (s32)(src[0x00] << 16 | (u16)src[0x10]);
        dst[0x01] = (1.0F/0x10000) * (s32)(src[0x01] << 16 | (u16)src[0x11]);
    #else
        dst[0x00] = (1.0F/0x10000) * (s32)(src[0x01] << 16 | (u16)src[0x11]);
        dst[0x01] = (1.0F/0x10000) * (s32)(src[0x00] << 16 | (u16)src[0x10]);
    #endif
        dst += 0x02;
        src += 0x02;
        cnt -= 0x02;
    }
    while (cnt > 0);
}

static void *gsp_addr(u32 addr)
{
    uint segment = addr >> 24 & 0x0F;
    u32  offset  = addr >>  0 & 0x00FFFFFF;
    return &gsp_addr_table[segment][offset];
}

static void gsp_texture_read_rgba16(void *buf, const void *addr, uint len)
{
#ifdef _3DS
    __WORDSWAP(buf, addr, len);
#else
    u8        *dst = buf;
    const u16 *src = addr;
    do
    {
        dst[0x00] = RGBA16_R(src[0x01]);
        dst[0x01] = RGBA16_G(src[0x01]);
        dst[0x02] = RGBA16_B(src[0x01]);
        dst[0x03] = RGBA16_A(src[0x01]);
        dst[0x04] = RGBA16_R(src[0x00]);
        dst[0x05] = RGBA16_G(src[0x00]);
        dst[0x06] = RGBA16_B(src[0x00]);
        dst[0x07] = RGBA16_A(src[0x00]);
        dst += 0x08;
        src += 0x02;
        len -= 0x04;
    }
    while (len > 0);
#endif
}

#if 0
static void gsp_texture_read_rgba32(void *buf, const void *addr, uint len)
{
    __BYTESWAP(buf, addr, len);
}
#endif

#ifndef APP_UNSM
/* CI 4 / 8 */
#endif

static void gsp_texture_read_ia4(void *buf, const void *addr, uint len)
{
    u8       *dst = buf;
    const u8 *src = addr;
    do
    {
    #ifdef _3DS
        dst[0x00] = IA4_IAH(src[0x03]);
        dst[0x01] = IA4_IAL(src[0x03]);
        dst[0x02] = IA4_IAH(src[0x02]);
        dst[0x03] = IA4_IAL(src[0x02]);
        dst[0x04] = IA4_IAH(src[0x01]);
        dst[0x05] = IA4_IAL(src[0x01]);
        dst[0x06] = IA4_IAH(src[0x00]);
        dst[0x07] = IA4_IAL(src[0x00]);
        dst += 0x08;
    #else
        dst[0x00] = dst[0x01] = dst[0x02] = IA4_IH(src[0x03]);
        dst[0x03]                         = IA4_AH(src[0x03]);
        dst[0x04] = dst[0x05] = dst[0x06] = IA4_IL(src[0x03]);
        dst[0x07]                         = IA4_AL(src[0x03]);
        dst[0x08] = dst[0x09] = dst[0x0A] = IA4_IH(src[0x02]);
        dst[0x0B]                         = IA4_AH(src[0x02]);
        dst[0x0C] = dst[0x0D] = dst[0x0E] = IA4_IL(src[0x02]);
        dst[0x0F]                         = IA4_AL(src[0x02]);
        dst[0x10] = dst[0x11] = dst[0x12] = IA4_IH(src[0x01]);
        dst[0x13]                         = IA4_AH(src[0x01]);
        dst[0x14] = dst[0x15] = dst[0x16] = IA4_IL(src[0x01]);
        dst[0x17]                         = IA4_AL(src[0x01]);
        dst[0x18] = dst[0x19] = dst[0x1A] = IA4_IH(src[0x00]);
        dst[0x1B]                         = IA4_AH(src[0x00]);
        dst[0x1C] = dst[0x1D] = dst[0x1E] = IA4_IL(src[0x00]);
        dst[0x1F]                         = IA4_AL(src[0x00]);
        dst += 0x20;
    #endif
        src += 0x04;
        len -= 0x04;
    }
    while (len > 0);
}

static void gsp_texture_read_ia8(void *buf, const void *addr, uint len)
{
#ifdef _3DS
    __BYTESWAP(buf, addr, len);
#else
    u8       *dst = buf;
    const u8 *src = addr;
    do
    {
        dst[0x00] = dst[0x01] = dst[0x02] = IA8_I(src[0x03]);
        dst[0x03]                         = IA8_A(src[0x03]);
        dst[0x04] = dst[0x05] = dst[0x06] = IA8_I(src[0x02]);
        dst[0x07]                         = IA8_A(src[0x02]);
        dst[0x08] = dst[0x09] = dst[0x0A] = IA8_I(src[0x01]);
        dst[0x0B]                         = IA8_A(src[0x01]);
        dst[0x0C] = dst[0x0D] = dst[0x0E] = IA8_I(src[0x00]);
        dst[0x0F]                         = IA8_A(src[0x00]);
        dst += 0x10;
        src += 0x04;
        len -= 0x04;
    }
    while (len > 0);
#endif
}

#ifdef APP_UNSM
static void gsp_texture_read_ia8_special(void *buf, const void *addr, uint len)
{
    u8       *dst = buf;
    const u8 *src = addr;
    do
    {
    #ifdef _3DS
        dst[0x00] = 0xF0 | (src[0x03] >> 4);
        dst[0x01] = 0xF0 | (src[0x02] >> 4);
        dst[0x02] = 0xF0 | (src[0x01] >> 4);
        dst[0x03] = 0xF0 | (src[0x00] >> 4);
        dst += 0x04;
    #else
        dst[0x00] = dst[0x01] = dst[0x02] = 0xFF;
        dst[0x03]                         = IA8_I(src[0x03]);
        dst[0x04] = dst[0x05] = dst[0x06] = 0xFF;
        dst[0x07]                         = IA8_I(src[0x02]);
        dst[0x08] = dst[0x09] = dst[0x0A] = 0xFF;
        dst[0x0B]                         = IA8_I(src[0x01]);
        dst[0x0C] = dst[0x0D] = dst[0x0E] = 0xFF;
        dst[0x0F]                         = IA8_I(src[0x00]);
        dst += 0x10;
    #endif
        src += 0x04;
        len -= 0x04;
    }
    while (len > 0);
}
#endif

static void gsp_texture_read_ia16(void *buf, const void *addr, uint len)
{
#ifdef _3DS
    __WORDSWAP(buf, addr, len);
#else
    u8       *dst = buf;
    const u8 *src = addr;
    do
    {
        dst[0x00] = dst[0x01] = dst[0x02] = src[0x03];
        dst[0x03]                         = src[0x02];
        dst[0x04] = dst[0x05] = dst[0x06] = src[0x01];
        dst[0x07]                         = src[0x00];
        dst += 0x08;
        src += 0x04;
        len -= 0x04;
    }
    while (len > 0);
#endif
}

#ifndef APP_UNSM
static void gsp_texture_read_i4(void *buf, const void *addr, uint len)
{
    u8       *dst = buf;
    const u8 *src = addr;
    do
    {
    #ifdef _3DS
        dst[0x00] = I4_IH(src[0x03]);
        dst[0x01] = I4_IL(src[0x03]);
        dst[0x02] = I4_IH(src[0x02]);
        dst[0x03] = I4_IL(src[0x02]);
        dst[0x04] = I4_IH(src[0x01]);
        dst[0x05] = I4_IL(src[0x01]);
        dst[0x06] = I4_IH(src[0x00]);
        dst[0x07] = I4_IL(src[0x00]);
        dst += 0x08;
    #else
        dst[0x00] = dst[0x01] = dst[0x02] = dst[0x03] = I4_IH(src[0x03]);
        dst[0x04] = dst[0x05] = dst[0x06] = dst[0x07] = I4_IL(src[0x03]);
        dst[0x08] = dst[0x09] = dst[0x0A] = dst[0x0B] = I4_IH(src[0x02]);
        dst[0x0C] = dst[0x0D] = dst[0x0E] = dst[0x0F] = I4_IL(src[0x02]);
        dst[0x10] = dst[0x11] = dst[0x12] = dst[0x13] = I4_IH(src[0x01]);
        dst[0x14] = dst[0x15] = dst[0x16] = dst[0x17] = I4_IL(src[0x01]);
        dst[0x18] = dst[0x19] = dst[0x1A] = dst[0x1B] = I4_IH(src[0x00]);
        dst[0x1C] = dst[0x1D] = dst[0x1E] = dst[0x1F] = I4_IL(src[0x00]);
        dst += 0x20;
    #endif
        src += 0x04;
        len -= 0x04;
    }
    while (len > 0);
}

static void gsp_texture_read_i8(void *buf, const void *addr, uint len)
{
#ifdef _3DS
    __BYTESWAP(buf, addr, len);
#else
    u8       *dst = buf;
    const u8 *src = addr;
    do
    {
        dst[0x00] = dst[0x01] = dst[0x02] = dst[0x03] = src[0x03];
        dst[0x04] = dst[0x05] = dst[0x06] = dst[0x07] = src[0x02];
        dst[0x08] = dst[0x09] = dst[0x0A] = dst[0x0B] = src[0x01];
        dst[0x0C] = dst[0x0D] = dst[0x0E] = dst[0x0F] = src[0x00];
        dst += 0x10;
        src += 0x04;
        len -= 0x04;
    }
    while (len > 0);
#endif
}
#endif

static void gsp_flush_mtxf_projection(void)
{
#ifdef GEKKO
    Mtx44 mtx;
    uint type;
    mtx[0][0] = MP[0][0];
    mtx[1][1] = MP[1][1];
    mtx[2][2] = MP[2][2];
    mtx[2][3] = MP[3][2];
    if (MP[3][3] != 0)
    {
        mtx[0][3] = MP[3][0];
        mtx[1][3] = MP[3][1];
        type = GX_ORTHOGRAPHIC;
    }
    else
    {
        mtx[0][2] = MP[2][0];
        mtx[1][2] = MP[2][1];
        type = GX_PERSPECTIVE;
    }
    GX_LoadProjectionMtx(mtx, type);
#else
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(&MP[0][0]);
#ifdef _3DS
    glTranslatef(gsp_depth, 0, 0);
#endif
#endif
}

static void gsp_flush_mtxf_modelview(void)
{
#ifdef GEKKO
    Mtx mtx;
    mtx[0][0] = MM[0][0];
    mtx[0][1] = MM[1][0];
    mtx[0][2] = MM[2][0];
    mtx[0][3] = MM[3][0];
    mtx[1][0] = MM[0][1];
    mtx[1][1] = MM[1][1];
    mtx[1][2] = MM[2][1];
    mtx[1][3] = MM[3][1];
    mtx[2][0] = MM[0][2];
    mtx[2][1] = MM[1][2];
    mtx[2][2] = MM[2][2];
    mtx[2][3] = MM[3][2];
    GX_LoadPosMtxImm(mtx, GX_PNMTX0);
#else
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(&MM[0][0]);
#endif
}

static void gsp_flush_viewport(void)
{
    s32 l;
    s32 r;
    s32 t;
    s32 b;
    if (gsp_rect == 0)
    {
        l = gsp_viewport.x - gsp_viewport.w;
        r = gsp_viewport.x + gsp_viewport.w;
        t = gsp_viewport.y - gsp_viewport.h;
        b = gsp_viewport.y + gsp_viewport.h;
    }
    else
    {
        l =    0;
        r = 1280;
        t =    0;
        b =  960;
    }
#ifdef GEKKO
    GX_SetViewport(
        (  l) * lib_video_w/1280.0F,
        (  t) * lib_video_h/ 960.0F,
        (r-l) * lib_video_w/1280.0F,
        (b-t) * lib_video_h/ 960.0F,
        0,
        1
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

static void gsp_flush_cull(void)
{
    if (gsp_rect == 0)
    {
    #ifdef GEKKO
        /* GX_SetCullMode(gsp_geometry_mode >> 12 & 0x03); */
        GX_SetCullMode(GX_CULL_NONE);
    #else
        switch (gsp_geometry_mode & G_CULL_BOTH)
        {
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
        }
    #endif
    }
    else
    {
    #ifdef GEKKO
        /* GX_SetCullMode(GX_CULL_BACK); */
        GX_SetCullMode(GX_CULL_NONE);
    #else
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
    #endif
    }
}

#define EN_ZR ((othermode_l & Z_CMP) != 0)
#define EN_ZW ((othermode_l & Z_UPD) != 0)
#define EN_BL \
    ((othermode_l & 0x0300) != CVG_DST_CLAMP && (othermode_l & FORCE_BL) != 0)
#define EN_DE ((othermode_l & 0x0C00) == ZMODE_DEC)
#define EN_AC ((othermode_l & CVG_X_ALPHA) != 0)
static void gsp_flush_rendermode(void)
{
    u32 othermode_l =
        gsp_cycle ? G_RM_TEX_EDGE | G_RM_TEX_EDGE2 : gsp_othermode_l;
#ifdef GEKKO
    GX_SetZMode(EN_ZR, GX_LEQUAL, EN_ZW);
    GX_SetBlendMode(
        EN_BL ? GX_BM_BLEND : GX_BM_NONE, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA,
        GX_LO_NOOP
    );
    GX_SetCoPlanar(EN_DE);
    GX_SetAlphaCompare(
        EN_AC ? GX_GEQUAL : GX_ALWAYS, 0x80, GX_AOP_AND, GX_ALWAYS, 0x00
    );
#else
    if (EN_ZR)
    {
        glEnable(GL_DEPTH_TEST);
    }
    else
    {
        glDisable(GL_DEPTH_TEST);
    }
    glDepthMask(EN_ZW);
    if (EN_BL)
    {
        glEnable(GL_BLEND);
    }
    else
    {
        glDisable(GL_BLEND);
    }
    if (EN_DE)
    {
        glPolygonOffset(-1, -2);
    }
    else
    {
        glPolygonOffset(0, 0);
    }
    if (EN_AC)
    {
        glEnable(GL_ALPHA_TEST);
    }
    else
    {
        glDisable(GL_ALPHA_TEST);
    }
#endif
}
#undef EN_ZR
#undef EN_ZW
#undef EN_BL
#undef EN_DE
#undef EN_AC

static void gsp_flush_scissor(void)
{
    s32 l = gsp_scissor_l;
    s32 r = gsp_scissor_r;
    s32 t = gsp_scissor_t;
    s32 b = gsp_scissor_b;
#ifdef GEKKO
    GX_SetScissor(
        (  l) * lib_video_w/1280,
        (  t) * lib_video_h/ 960,
        (r-l) * lib_video_w/1280,
        (b-t) * lib_video_h/ 960
    );
#else
    glScissor(
        (    l) * lib_video_w/1280,
        (960-b) * lib_video_h/ 960,
        (  r-l) * lib_video_w/1280,
        (  b-t) * lib_video_h/ 960
    );
#endif
}

#define EN_TX ((gsp_rect == 0 && gsp_texture_enabled) || gsp_rect == 1)
static void gsp_flush_texture_enabled(void)
{
#ifdef GEKKO
    GX_SetTevOp(GX_TEVSTAGE0, EN_TX ? GX_MODULATE : GX_PASSCLR);
#else
    if (EN_TX)
    {
        glEnable(GL_TEXTURE_2D);
    }
    else
    {
        glDisable(GL_TEXTURE_2D);
    }
#endif
}
#undef EN_TX

static void gsp_flush_texture(void)
{
#ifdef GEKKO
    /* todo: texture */
#else
    void  *addr   = gsp_texture_addr;
    GLuint filter = gsp_texture_filter;
    uint i;
    for (i = 0; i < gsp_texture; i++)
    {
        struct texture_t *texture = &gsp_texture_table[i];
        GLuint *name = &gsp_texture_name_table[i];
        if (texture->addr == addr && texture->filter == filter)
        {
            glBindTexture(GL_TEXTURE_2D, *name);
            return;
        }
    }
    if (gsp_texture < GSP_TEXTURE_LEN)
    {
        struct texture_t *texture = &gsp_texture_table[gsp_texture];
        GLuint *name = &gsp_texture_name_table[i];
        void (*texture_read)(void *, const void *, uint);
        texture->addr   = addr;
        texture->filter = filter;
        glFlush();
        glGenTextures(1, name);
        glBindTexture(GL_TEXTURE_2D, *name);
        texture_read = gsp_texture_read_table[gsp_texture_fmt];
        if (texture_read != NULL)
        {
            GLint fs = gsp_texture_flag_table[gsp_texture_flag[0]];
            GLint ft = gsp_texture_flag_table[gsp_texture_flag[1]];
            texture_read(gsp_texture_buf, addr, gsp_texture_len);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, fs);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, ft);
            glTexImage2D(
                GL_TEXTURE_2D, 0,
            #ifdef _3DS
                gsp_texture_fmt_table[gsp_texture_fmt],
            #else
                GL_RGBA8,
            #endif
                gsp_texture_size[0], gsp_texture_size[1],
                0, GL_RGBA, GL_UNSIGNED_BYTE, gsp_texture_buf
            );
            gsp_texture++;
        }
        else
        {
        #ifdef _DEBUG
        #ifndef APP_UNK4
            fprintf(
                stderr, "warning: unknown texture fmt %02X / %db\n",
                gsp_texture_fmt >> 2, 4 << (gsp_texture_fmt & 0x03)
            );
        #endif
        #endif
        }
    }
    else
    {
    #ifdef _DEBUG
        fprintf(stderr, "warning: texture over\n");
    #endif
    }
#endif
}

static void gsp_flush_triangles(void)
{
#ifndef GSP_LEGACY
    if (gsp_output_count > 0)
    {
    #ifdef GEKKO
        uint i;
        GX_SetArray(GX_VA_POS,  gsp_output_vtx, sizeof(s16)*3);
        GX_SetArray(GX_VA_TEX0, gsp_output_txc, sizeof(f32)*2);
        GX_SetArray(GX_VA_CLR0, gsp_output_col, sizeof(u8)*4);
        GX_Begin(GX_TRIANGLES, GX_VTXFMT0, gsp_output_count);
        for (i = 0; i < gsp_output_count; i++)
        {
            GX_Position1x16(i);
            GX_Color1x16(i);
            GX_TexCoord1x16(i);
        }
        GX_End();
    #else
        glVertexPointer(3, GL_SHORT, 0, gsp_output_vtx);
        glTexCoordPointer(2, GL_FLOAT, 0, gsp_output_txc);
        glColorPointer(4, GL_UNSIGNED_BYTE, 0, gsp_output_col);
        glDrawArrays(GL_TRIANGLES, 0, gsp_output_count);
    #endif
        gsp_output_vtx = gsp_output_v;
        gsp_output_txc = gsp_output_t;
        gsp_output_col = gsp_output_c;
        gsp_output_count = 0;
    }
#endif
}

static void gsp_flush(void)
{
    gsp_flush_triangles();
    if (gsp_change & CHANGE_MTXF_PROJECTION)
    {
        gsp_flush_mtxf_projection();
    }
    if (gsp_change & CHANGE_MTXF_MODELVIEW)
    {
        gsp_flush_mtxf_modelview();
    }
    if (gsp_change & CHANGE_VIEWPORT)
    {
        gsp_flush_viewport();
    }
    if (gsp_change & CHANGE_CULL)
    {
        gsp_flush_cull();
    }
    if (gsp_change & CHANGE_RENDERMODE)
    {
        gsp_flush_rendermode();
    }
    if (gsp_change & CHANGE_SCISSOR)
    {
        gsp_flush_scissor();
    }
    if (gsp_change & CHANGE_TEXTURE_ENABLED)
    {
        gsp_flush_texture_enabled();
    }
    if (gsp_change & CHANGE_TEXTURE)
    {
        gsp_flush_texture();
    }
    gsp_change = 0;
}

static void gsp_combine_cc_0(u8 *col, unused struct vtxf_t *vtxf)
{
    col[0] = 0x00;
    col[1] = 0x00;
    col[2] = 0x00;
}

static void gsp_combine_cc_1(u8 *col, unused struct vtxf_t *vtxf)
{
    col[0] = 0xFF;
    col[1] = 0xFF;
    col[2] = 0xFF;
}

static void gsp_combine_cc_shade(u8 *col, struct vtxf_t *vtxf)
{
    col[0] = vtxf->shade[0];
    col[1] = vtxf->shade[1];
    col[2] = vtxf->shade[2];
}

static void gsp_combine_cc_prim(u8 *col, unused struct vtxf_t *vtxf)
{
    col[0] = gsp_prim[0];
    col[1] = gsp_prim[1];
    col[2] = gsp_prim[2];
}

static void gsp_combine_cc_env(u8 *col, unused struct vtxf_t *vtxf)
{
    col[0] = gsp_env[0];
    col[1] = gsp_env[1];
    col[2] = gsp_env[2];
}

#ifdef APP_UNSM
static void gsp_combine_cc_shade_env(u8 *col, struct vtxf_t *vtxf)
{
    col[0] = vtxf->shade[0] * gsp_env[0] / 0x100;
    col[1] = vtxf->shade[1] * gsp_env[1] / 0x100;
    col[2] = vtxf->shade[2] * gsp_env[2] / 0x100;
}
#endif

#ifdef APP_UNK4
static void gsp_combine_cc_shade_prim(u8 *col, struct vtxf_t *vtxf)
{
    col[0] = vtxf->shade[0] * gsp_prim[0] / 0x100;
    col[1] = vtxf->shade[1] * gsp_prim[1] / 0x100;
    col[2] = vtxf->shade[2] * gsp_prim[2] / 0x100;
}

static void gsp_combine_cc_prim_env(u8 *col, unused struct vtxf_t *vtxf)
{
    col[0] = gsp_prim[0] * gsp_env[0] / 0x100;
    col[1] = gsp_prim[1] * gsp_env[1] / 0x100;
    col[2] = gsp_prim[2] * gsp_env[2] / 0x100;
}

static void gsp_combine_cc_prim_env_shade_env(u8 *col, struct vtxf_t *vtxf)
{
    col[0] = (gsp_prim[0]-gsp_env[0]) * vtxf->shade[0]/0x100 + gsp_env[0];
    col[1] = (gsp_prim[1]-gsp_env[1]) * vtxf->shade[1]/0x100 + gsp_env[1];
    col[2] = (gsp_prim[2]-gsp_env[2]) * vtxf->shade[2]/0x100 + gsp_env[2];
}
#endif

#ifdef GSP_FOG
static void gsp_combine_cc_fog(u8 *col, unused struct vtxf_t *vtxf)
{
    col[0] = gsp_fog[0];
    col[1] = gsp_fog[1];
    col[2] = gsp_fog[2];
}
#endif

#ifdef APP_UNSM
static void gsp_combine_cc_special1(u8 *col, struct vtxf_t *vtxf)
{
    if (gsp_texture_enabled)
    {
        col[0] = 0xFF;
        col[1] = 0xFF;
        col[2] = 0xFF;
    }
    else
    {
        col[0] = vtxf->shade[0];
        col[1] = vtxf->shade[1];
        col[2] = vtxf->shade[2];
    }
}

static void gsp_combine_cc_special2(u8 *col, struct vtxf_t *vtxf)
{
    if (gsp_texture_enabled)
    {
        col[0] = gsp_prim[0];
        col[1] = gsp_prim[1];
        col[2] = gsp_prim[2];
    }
    else
    {
        col[0] = vtxf->shade[0];
        col[1] = vtxf->shade[1];
        col[2] = vtxf->shade[2];
    }
}
#endif

#ifdef APP_UNK4
static void gsp_combine_cc_special3(u8 *col, struct vtxf_t *vtxf)
{
    if (gsp_texture_enabled)
    {
        col[0] = vtxf->shade[0];
        col[1] = vtxf->shade[1];
        col[2] = vtxf->shade[2];
    }
    else
    {
        col[0] = gsp_env[0];
        col[1] = gsp_env[1];
        col[2] = gsp_env[2];
    }
}

static void gsp_combine_cc_special4(u8 *col, unused struct vtxf_t *vtxf)
{
    if (gsp_texture_enabled)
    {
        col[0] = gsp_prim[0];
        col[1] = gsp_prim[1];
        col[2] = gsp_prim[2];
    }
    else
    {
        col[0] = gsp_env[0];
        col[1] = gsp_env[1];
        col[2] = gsp_env[2];
    }
}

static void gsp_combine_cc_special5(u8 *col, struct vtxf_t *vtxf)
{
    gsp_combine_cc_special4(col, vtxf);
    col[0] = col[0] * vtxf->shade[0] / 0x100;
    col[1] = col[1] * vtxf->shade[1] / 0x100;
    col[2] = col[2] * vtxf->shade[2] / 0x100;
}
#endif

static void gsp_combine_ac_0(u8 *col, unused struct vtxf_t *vtxf)
{
    col[3] = 0x00;
}

static void gsp_combine_ac_1(u8 *col, unused struct vtxf_t *vtxf)
{
    col[3] = 0xFF;
}

static void gsp_combine_ac_shade(u8 *col, struct vtxf_t *vtxf)
{
    col[3] = vtxf->shade[3];
}

static void gsp_combine_ac_prim(u8 *col, unused struct vtxf_t *vtxf)
{
    col[3] = gsp_prim[3];
}

#ifdef APP_UNSM
static void gsp_combine_ac_env(u8 *col, unused struct vtxf_t *vtxf)
{
    col[3] = gsp_env[3];
}

static void gsp_combine_ac_shade_env(u8 *col, unused struct vtxf_t *vtxf)
{
    col[3] = vtxf->shade[3] * gsp_env[3] / 0x100;
}
#endif

#ifdef GSP_FOG
static void gsp_combine_ac_fog(u8 *col, struct vtxf_t *vtxf)
{
    col[3] = vtxf->shade[3];
}
#endif

static void gsp_write_triangle_init(u8 *t)
{
    uint i;
    if (gsp_change != 0)
    {
        gsp_flush();
    }
#ifdef GSP_LEGACY
    glBegin(GL_TRIANGLES);
#endif
    for (i = 0; i < 3; i++)
    {
        struct vtx_t  *vtx  = &gsp_vtx_buf[t[i]];
        struct vtxf_t *vtxf = &gsp_vtxf_buf[t[i]];
        u8 col[4];
        gsp_combine_cc(col, vtxf);
        gsp_combine_ac(col, vtxf);
    #ifdef GSP_LEGACY
        glColor4ub(col[0], col[1], col[2], col[3]);
        glTexCoord2f(
            vtxf->u*gsp_texture_tscale[0], vtxf->v*gsp_texture_tscale[1]
        );
        glVertex3s(vtx->x, vtx->y, vtx->z);
    #else
        gsp_output_v[0] = vtx->x;
        gsp_output_v[1] = vtx->y;
        gsp_output_v[2] = vtx->z;
        gsp_output_t[0] = vtxf->u*gsp_texture_tscale[0];
        gsp_output_t[1] = vtxf->v*gsp_texture_tscale[1];
        gsp_output_c[0] = col[0];
        gsp_output_c[1] = col[1];
        gsp_output_c[2] = col[2];
        gsp_output_c[3] = col[3];
        gsp_output_v += 3;
        gsp_output_t += 2;
        gsp_output_c += 4;
        gsp_output_total++;
        gsp_output_count++;
    #endif
    }
#ifdef GSP_LEGACY
    glEnd();
#endif
}

#ifdef _3DS
#ifndef GSP_LEGACY
static void gsp_write_triangle_stub(unused u8 *t)
{
    if (gsp_change != 0)
    {
        gsp_flush();
    }
    gsp_output_v += 3*3;
    gsp_output_t += 3*2;
    gsp_output_c += 3*4;
    gsp_output_total += 3;
    gsp_output_count += 3;
}
#endif
#endif

static void gsp_triangle_default(u8 *t)
{
#ifndef GSP_LEGACY
    if (gsp_output_total < GSP_OUTPUT_LEN)
#endif
    {
        gsp_write_triangle(t);
    }
#ifndef GSP_LEGACY
    else
    {
    #ifdef _DEBUG
        fprintf(stderr, "warning: output over\n");
    #endif
    }
#endif
}

#ifdef GSP_FOG
static void gsp_triangle_fog(u8 *t)
{
#ifndef GSP_LEGACY
    if (gsp_output_total < GSP_OUTPUT_LEN-1)
#endif
    {
        u8 sa0;
        u8 sa1;
        u8 sa2;
        gsp_write_triangle(t);
        sa0 = gsp_vtxf_buf[t[0]].shade[3];
        sa1 = gsp_vtxf_buf[t[1]].shade[3];
        sa2 = gsp_vtxf_buf[t[2]].shade[3];
        if (sa0 > 0x00 || sa1 > 0x00 || sa2 > 0x00)
        {
            u32 othermode_l;
            u8  texture_enabled;
            void (*combine_cc)(u8 *, struct vtxf_t *);
            void (*combine_ac)(u8 *, struct vtxf_t *);
            othermode_l = gsp_othermode_l;
            gsp_g_setothermode_l(
                0xB900031D, G_RM_AA_ZB_XLU_DECAL | G_RM_AA_ZB_XLU_DECAL2
            );
            texture_enabled = gsp_texture_enabled;
            combine_cc = gsp_combine_cc;
            combine_ac = gsp_combine_ac;
            gsp_texture_enabled = G_OFF;
            gsp_combine_cc = gsp_combine_cc_fog;
            gsp_combine_ac = gsp_combine_ac_fog;
            gsp_change |= CHANGE_TEXTURE_ENABLED;
            gsp_write_triangle(t);
            gsp_othermode_l = othermode_l;
            gsp_texture_enabled = texture_enabled;
            gsp_combine_cc = combine_cc;
            gsp_combine_ac = combine_ac;
            gsp_change |= CHANGE_RENDERMODE | CHANGE_TEXTURE_ENABLED;
        }
    }
#ifndef GSP_LEGACY
    else
    {
    #ifdef _DEBUG
        fprintf(stderr, "warning: output over\n");
    #endif
    }
#endif
}
#endif

static void gsp_triangle_special(u8 *t)
{
#ifndef GSP_LEGACY
    if (gsp_output_total < GSP_OUTPUT_LEN-1)
#endif
    {
        u8  texture_enabled;
        u32 othermode_l;
        texture_enabled = gsp_texture_enabled;
        gsp_texture_enabled = G_OFF;
        gsp_change |= CHANGE_TEXTURE_ENABLED;
        gsp_write_triangle(t);
        othermode_l = gsp_othermode_l;
        gsp_g_setothermode_l(
            0xB900031D, G_RM_AA_ZB_XLU_DECAL | G_RM_AA_ZB_XLU_DECAL2
        );
        gsp_texture_enabled = texture_enabled;
        gsp_change |= CHANGE_TEXTURE_ENABLED;
        gsp_write_triangle(t);
        gsp_othermode_l = othermode_l;
        gsp_change |= CHANGE_RENDERMODE;
    }
#ifndef GSP_LEGACY
    else
    {
    #ifdef _DEBUG
        fprintf(stderr, "warning: output over\n");
    #endif
    }
#endif
}

static void gsp_flush_texrect(void)
{
    f32  uh;
    f32  vh;
    uint i;
    u8   t[3];
    if (gsp_rect == 0)
    {
        gsp_mtxf_projection++;
        gsp_mtxf_modelview++;
        mtxf_identity(MM);
        gsp_change |=
            CHANGE_MTXF_MODELVIEW | CHANGE_VIEWPORT |
            CHANGE_CULL | CHANGE_RENDERMODE | CHANGE_TEXTURE_ENABLED;
    }
    if (gsp_rect != 1)
    {
        gsp_rect = 1;
        mtxf_ortho(
            MP,
        #ifdef LIB_DYNRES
            4*lib_viewport_l, 4*lib_viewport_r,
        #else
            0, 1280,
        #endif
            960, 0, -1, 1
        );
        gsp_change |= CHANGE_MTXF_PROJECTION;
    }
    if (gsp_cycle)
    {
        gsp_texrect_xh &= ~3;
        gsp_texrect_yh &= ~3;
        gsp_texrect_xh += 4;
        gsp_texrect_yh += 4;
        gsp_texrect_dsdx = 32/4.0F;
    }
    else if (gsp_texture_filter != GL_NEAREST)
    {
        gsp_texrect_ul += 32*0.5F;
        gsp_texrect_vl += 32*0.5F;
    }
    uh = gsp_texrect_ul + gsp_texrect_dsdx*(gsp_texrect_xh-gsp_texrect_xl);
    vh = gsp_texrect_vl + gsp_texrect_dtdy*(gsp_texrect_yh-gsp_texrect_yl);
    if (gsp_texrect_flip)
    {
        gsp_vtxf_buf[0].u = uh;
        gsp_vtxf_buf[0].v = gsp_texrect_vl;
        gsp_vtxf_buf[2].u = gsp_texrect_ul;
        gsp_vtxf_buf[2].v = vh;
    }
    else
    {
        gsp_vtxf_buf[0].u = gsp_texrect_ul;
        gsp_vtxf_buf[0].v = vh;
        gsp_vtxf_buf[2].u = uh;
        gsp_vtxf_buf[2].v = gsp_texrect_vl;
    }
    gsp_vtxf_buf[1].u = uh;
    gsp_vtxf_buf[1].v = vh;
    gsp_vtxf_buf[3].u = gsp_texrect_ul;
    gsp_vtxf_buf[3].v = gsp_texrect_vl;
    for (i = 0; i < 4; i++)
    {
        struct vtx_t  *vtx  = &gsp_vtx_buf[i];
        struct vtxf_t *vtxf = &gsp_vtxf_buf[i];
        vtx->x = i == 0 || i == 3 ? gsp_texrect_xl : gsp_texrect_xh;
        vtx->y = i == 2 || i == 3 ? gsp_texrect_yl : gsp_texrect_yh;
        vtx->z = 0;
        vtxf->shade[0] = 0xFF;
        vtxf->shade[1] = 0xFF;
        vtxf->shade[2] = 0xFF;
        vtxf->shade[3] = 0xFF;
    }
    t[0] = 0;
    t[1] = 1;
    t[2] = 2;
    gsp_write_triangle(t);
    t[0] = 0;
    t[1] = 2;
    t[2] = 3;
    gsp_write_triangle(t);
}

static void gsp_flush_rect(void)
{
    if (gsp_rect != 0)
    {
        gsp_rect = 0;
        gsp_mtxf_projection--;
        gsp_mtxf_modelview--;
        gsp_change |=
            CHANGE_MTXF_PROJECTION | CHANGE_MTXF_MODELVIEW | CHANGE_VIEWPORT |
            CHANGE_CULL | CHANGE_RENDERMODE | CHANGE_TEXTURE_ENABLED;
    }
}

static void gsp_start(void *ucode, u32 *dl)
{
    u32 i;
#ifdef GSP_F3D
    (void)ucode;
#endif
#ifdef GSP_F3DEX2
    i = (u8 *)ucode - cpu_dram;
    switch (i)
    {
        case 0x00039E90:
            /* printf("notice: using F3DEX2\n"); */
            goto meme;
        case 0x0003B220:
            /* printf("notice: using L3DEX2\n"); */
        meme:
            memcpy(&gsp_table[0x01], gsp_table_3d, sizeof(gsp_table_3d));
            gsp_table[G_MTX]     = gsp_g_mtx;
            gsp_table[G_MOVEMEM] = gsp_g_movemem;
            gsp_table[G_TEXRECT] = gsp_g_texrect;
            break;
        case 0x0003C3B0:
            /* printf("notice: using S2DEX2\n"); */
            memcpy(&gsp_table[0x01], gsp_table_2d, sizeof(gsp_table_2d));
            gsp_table[G_OBJ_RECTANGLE_R] = gsp_g_obj_rectangle_r;
            gsp_table[G_OBJ_MOVEMEM]     = gsp_g_obj_movemem;
            gsp_table[G_RDPHALF_0]       = gsp_g_rdphalf_0;
            break;
        default:
            fprintf(stderr, "error: unknown ucode 0x%08" FMT_X "\n", i);
            exit(EXIT_FAILURE);
            break;
    }
#endif
#ifndef GSP_LEGACY
    gsp_output_vtx = gsp_output_v = gsp_output_vtx_buf;
    gsp_output_txc = gsp_output_t = gsp_output_txc_buf;
    gsp_output_col = gsp_output_c = gsp_output_col_buf;
    gsp_output_total = 0;
    gsp_output_count = 0;
#endif
    gsp_mtxf_projection = gsp_mtxf_projection_stack;
    gsp_mtxf_modelview  = gsp_mtxf_modelview_stack;
    for (i = 0; i < lenof(gsp_addr_table); i++)
    {
        gsp_addr_table[i] = cpu_dram;
    }
    gsp_dl_stack[0] = dl;
    gsp_dl_index = 0;
    gsp_lookat = false;
    gsp_light_new = false;
    gsp_rect = 0;
    gsp_change = CHANGE_MTXF_PROJECTION | CHANGE_MTXF_MODELVIEW;
}

#if 0
/* 0x00 G_SPNOOP */
static void gsp_g_spnoop(unused u32 w0, unused u32 w1)
{
}
#endif

#ifdef GSP_F3D
/* 0x01 G_MTX */
#include "gsp/g_mtx.c"

/* 0x03 G_MOVEMEM */
#include "gsp/g_movemem.c"

/* 0x04 G_VTX */
#include "gsp/g_vtx.c"

/* 0x06 G_DL */
#include "gsp/g_dl.c"

#ifdef GSP_F3D_20D

#if 0
/* 0xB1 G_RDPHALF_CONT */
static void gsp_g_rdphalf_cont(unused u32 w0, unused u32 w1)
{
}
#endif

/* 0xB2 G_RDPHALF_2 */
#include "gsp/g_rdphalf_2.c"

/* 0xB3 G_RDPHALF_1 */
#include "gsp/g_rdphalf_1.c"

#ifdef _DEBUG
/* 0xB4 G_PERSPNORM */
static void gsp_g_perspnorm(unused u32 w0, unused u32 w1)
{
}
#endif

#else

#if 0
/* 0xB2 G_RDPHALF_CONT */
static void gsp_g_rdphalf_cont(unused u32 w0, unused u32 w1)
{
}
#endif

/* 0xB3 G_RDPHALF_2 */
#include "gsp/g_rdphalf_2.c"

/* 0xB4 G_RDPHALF_1 */
#include "gsp/g_rdphalf_1.c"

#endif

/* 0xB6 G_CLEARGEOMETRYMODE */
#include "gsp/g_cleargeometrymode.c"

/* 0xB7 G_SETGEOMETRYMODE */
#include "gsp/g_setgeometrymode.c"

/* 0xB8 G_ENDDL */
#include "gsp/g_enddl.c"

/* 0xB9 G_SETOTHERMODE_L */
#include "gsp/g_setothermode_l.c"

/* 0xBA G_SETOTHERMODE_H */
#include "gsp/g_setothermode_h.c"

/* 0xBB G_TEXTURE */
#include "gsp/g_texture.c"

/* 0xBC G_MOVEWORD */
#include "gsp/g_moveword.c"

/* 0xBD G_POPMTX */
#include "gsp/g_popmtx.c"

#if 0
/* 0xBE G_CULLDL */
#include "gsp/g_culldl.c"
#endif

/* 0xBF G_TRI1 */
#include "gsp/g_tri1.c"

#if 0
/* 0xC0 G_NOOP */
static void gsp_g_noop(unused u32 w0, unused u32 w1)
{
}
#endif
#endif

#ifdef GSP_F3DEX2
/* 0x01 G_VTX */
#include "gsp/g_vtx.c"

/* 0x02 G_MODIFYVTX */
#include "gsp/g_modifyvtx.c"

/* 0x03 G_CULLDL */
#include "gsp/g_culldl.c"

/* 0x04 G_BRANCH_Z */
static void gsp_g_branch_z(unused u32 w0, unused u32 w1)
{
    /* todo: branchz */
    gsp_dl_stack[gsp_dl_index] = gsp_addr(gsp_rdphalf_1);
}

/* 0x05 G_TRI1 */
#include "gsp/g_tri1.c"

/* 0x06 G_TRI2 */
#include "gsp/g_tri2.c"

/* 0x07 G_QUAD */
static void gsp_g_quad(unused u32 w0, unused u32 w1)
{
    puts("G_QUAD");
    exit(EXIT_FAILURE);
}

/* 0xD3 G_SPECIAL_3 */
static void gsp_g_special_3(unused u32 w0, unused u32 w1)
{
    puts("G_SPECIAL_3");
    exit(EXIT_FAILURE);
}

/* 0xD4 G_SPECIAL_2 */
static void gsp_g_special_2(unused u32 w0, unused u32 w1)
{
    puts("G_SPECIAL_2");
    exit(EXIT_FAILURE);
}

/* 0xD5 G_SPECIAL_1 */
#include "gsp/g_special_1.c"

/* 0xD6 G_DMA_IO */
static void gsp_g_dma_io(unused u32 w0, unused u32 w1)
{
    puts("G_DMA_IO");
    exit(EXIT_FAILURE);
}

/* 0xD7 G_TEXTURE */
#include "gsp/g_texture.c"

/* 0xD8 G_POPMTX */
#include "gsp/g_popmtx.c"

/* 0xD9 G_GEOMETRYMODE */
#include "gsp/g_geometrymode.c"

/* 0xDA G_MTX */
#include "gsp/g_mtx.c"

/* 0xDB G_MOVEWORD */
#include "gsp/g_moveword.c"

/* 0xDC G_MOVEMEM */
#include "gsp/g_movemem.c"

/* 0xDD G_LOAD_UCODE */
static void gsp_g_load_ucode(unused u32 w0, u32 w1)
{
    gsp_flush_triangles();
    gsp_start(gsp_addr(w1), gsp_dl_stack[gsp_dl_index]);
}

/* 0xDE G_DL */
#include "gsp/g_dl.c"

/* 0xDF G_ENDDL */
#include "gsp/g_enddl.c"

#if 0
/* 0xE0 G_NOOP */
static void gsp_g_noop(unused u32 w0, unused u32 w1)
{
}
#endif

/* 0xE1 G_RDPHALF_1 */
#include "gsp/g_rdphalf_1.c"

/* 0xE2 G_SETOTHERMODE_L */
#include "gsp/g_setothermode_l.c"

/* 0xE3 G_SETOTHERMODE_H */
#include "gsp/g_setothermode_h.c"
#endif

/* 0xE4 G_TEXRECT */
static void gsp_g_texrect(u32 w0, u32 w1)
{
    gsp_texrect_xh = (s16)(w0 >> 8) >> 4;
    gsp_texrect_yh = (s16)(w0 << 4) >> 4;
    gsp_texrect_xl = (s16)(w1 >> 8) >> 4;
    gsp_texrect_yl = (s16)(w1 << 4) >> 4;
    gsp_texrect_flip = false;
}

/* 0xE5 G_TEXRECTFLIP */
static void gsp_g_texrectflip(u32 w0, u32 w1)
{
    gsp_texrect_xh = (s16)(w0 >> 8) >> 4;
    gsp_texrect_yh = (s16)(w0 << 4) >> 4;
    gsp_texrect_xl = (s16)(w1 >> 8) >> 4;
    gsp_texrect_yl = (s16)(w1 << 4) >> 4;
    gsp_texrect_flip = true;
}

#ifdef _DEBUG
/* 0xE6 G_RDPLOADSYNC */
static void gsp_g_rdploadsync(unused u32 w0, unused u32 w1)
{
}

/* 0xE7 G_RDPPIPESYNC */
static void gsp_g_rdppipesync(unused u32 w0, unused u32 w1)
{
}

/* 0xE8 G_RDPTILESYNC */
static void gsp_g_rdptilesync(unused u32 w0, unused u32 w1)
{
}

/* 0xE9 G_RDPFULLSYNC */
static void gsp_g_rdpfullsync(unused u32 w0, unused u32 w1)
{
}
#endif

#if 0
/* 0xEA G_SETKEYGB */
static void gsp_g_setkeygb(unused u32 w0, unused u32 w1)
{
}

/* 0xEB G_SETKEYR */
static void gsp_g_setkeyr(unused u32 w0, unused u32 w1)
{
}

/* 0xEC G_SETCONVERT */
static void gsp_g_setconvert(unused u32 w0, unused u32 w1)
{
}
#endif

/* 0xED G_SETSCISSOR */
static void gsp_g_setscissor(u32 w0, u32 w1)
{
    gsp_scissor_l = w0 >> 12 & 0x0FFF;
    gsp_scissor_t = w0 >>  0 & 0x0FFF;
    gsp_scissor_r = w1 >> 12 & 0x0FFF;
    gsp_scissor_b = w1 >>  0 & 0x0FFF;
    gsp_change |= CHANGE_SCISSOR;
}

#ifdef APP_UNK4
/* 0xEE G_SETPRIMDEPTH */
static void gsp_g_setprimdepth(unused u32 w0, unused u32 w1)
{
    /* todo: primdepth (?) */
}
#endif

#if 0
/* 0xEF G_RDPSETOTHERMODE */
static void gsp_g_rdpsetothermode(u32 w0, u32 w1)
{
    gsp_othermode_h = w0;
    gsp_othermode_l = w1;
    gsp_change |= CHANGE_RENDERMODE;
}
#endif

#ifdef APP_UNK4
/* 0xF0 G_LOADTLUT */
static void gsp_g_loadtlut(unused u32 w0, unused u32 w1)
{
    /* todo: CI */
}
#endif

#ifdef GSP_F3DEX2
/* 0xF1 G_RDPHALF_2 */
#include "gsp/g_rdphalf_2.c"
#endif

/* 0xF2 G_SETTILESIZE */
static void gsp_g_settilesize(u32 w0, u32 w1)
{
    if ((w0 & 0x00FFFFFF) == 0)
    {
        gsp_texture_size[0] = (w1 >> 14 & 0x03FF) + 1;
        gsp_texture_size[1] = (w1 >>  2 & 0x03FF) + 1;
        gsp_texture_tscale[0] = (1.0F/32) / gsp_texture_size[0];
        gsp_texture_tscale[1] = (1.0F/32) / gsp_texture_size[1];
    }
    gsp_change |= CHANGE_TEXTURE;
}

/* 0xF3 G_LOADBLOCK */
static void gsp_g_loadblock(unused u32 w0, unused u32 w1)
{
    gsp_texture_len = ((w1 >> 12 & 0x0FFF)+1) << (gsp_texture_fmt & 0x03) >> 1;
    gsp_change |= CHANGE_TEXTURE;
}

/* 0xF4 G_LOADTILE */
static void gsp_g_loadtile(unused u32 w0, unused u32 w1)
{
    gsp_change |= CHANGE_TEXTURE;
}

/* 0xF5 G_SETTILE */
static void gsp_g_settile(u32 w0, u32 w1)
{
    u32 tile;
    gsp_texture_fmt = w0 >> 19 & 0x1F;
    tile            = w1 >> 24 & 0x07;
    if (tile == G_TX_RENDERTILE)
    {
        gsp_texture_flag[1]  = w1 >> 18 & 0x03;
        gsp_texture_shift[1] = w1 >> 10 & 0x07;
        gsp_texture_flag[0]  = w1 >>  8 & 0x03;
        gsp_texture_shift[0] = w1 >>  0 & 0x07;
    }
    gsp_change |= CHANGE_TEXTURE;
}

/* 0xF6 G_FILLRECT */
static void gsp_g_fillrect(u32 w0, u32 w1)
{
    if (gsp_fill != 0xFFFC)
    {
        u32 xh;
        u32 yh;
        u32 xl;
        u32 yl;
        u32 r;
        u32 g;
        u32 b;
        u32 i;
        u8  t[3];
        if (gsp_rect == 0)
        {
            gsp_mtxf_projection++;
            gsp_mtxf_modelview++;
            mtxf_identity(MM);
            gsp_change |=
                CHANGE_MTXF_MODELVIEW | CHANGE_CULL |
                CHANGE_RENDERMODE | CHANGE_TEXTURE_ENABLED;
        }
        if (gsp_rect != 2)
        {
            gsp_rect = 2;
            mtxf_ortho(MP, 0, 320, 240, 0, -1, 1);
            gsp_change |= CHANGE_MTXF_PROJECTION;
        }
        xh = w0 >> 14 & 0x03FF;
        yh = w0 >>  2 & 0x03FF;
        xl = w1 >> 14 & 0x03FF;
        yl = w1 >>  2 & 0x03FF;
        r = RGBA16_R(gsp_fill);
        g = RGBA16_G(gsp_fill);
        b = RGBA16_B(gsp_fill);
        if (gsp_cycle)
        {
            xh++;
            yh++;
        }
        for (i = 0; i < 4; i++)
        {
            struct vtx_t  *vtx  = &gsp_vtx_buf[i];
            struct vtxf_t *vtxf = &gsp_vtxf_buf[i];
            vtx->x = i == 0 || i == 3 ? xl : xh;
            vtx->y = i == 2 || i == 3 ? yl : yh;
            vtx->z = 0;
            vtxf->shade[0] = r;
            vtxf->shade[1] = g;
            vtxf->shade[2] = b;
            vtxf->shade[3] = 0xFF;
        }
        t[0] = 0;
        t[1] = 1;
        t[2] = 2;
        gsp_write_triangle(t);
        t[0] = 0;
        t[1] = 2;
        t[2] = 3;
        gsp_write_triangle(t);
    }
}

/* 0xF7 G_SETFILLCOLOR */
static void gsp_g_setfillcolor(unused u32 w0, u32 w1)
{
    gsp_fill = w1 >> 16;
}

/* 0xF8 G_SETFOGCOLOR */
static void gsp_g_setfogcolor(unused u32 w0, u32 w1)
{
    gsp_fog[0] = w1 >> 24;
    gsp_fog[1] = w1 >> 16;
    gsp_fog[2] = w1 >>  8;
    gsp_fog[3] = w1 >>  0;
}

/* 0xF9 G_SETBLENDCOLOR */
static void gsp_g_setblendcolor(unused u32 w0, unused u32 w1)
{
#if 0
    gsp_blend[0] = w1 >> 24;
    gsp_blend[1] = w1 >> 16;
    gsp_blend[2] = w1 >>  8;
    gsp_blend[3] = w1 >>  0;
#endif
}

/* 0xFA G_SETPRIMCOLOR */
static void gsp_g_setprimcolor(unused u32 w0, u32 w1)
{
    gsp_prim[0] = w1 >> 24;
    gsp_prim[1] = w1 >> 16;
    gsp_prim[2] = w1 >>  8;
    gsp_prim[3] = w1 >>  0;
}

/* 0xFB G_SETENVCOLOR */
static void gsp_g_setenvcolor(unused u32 w0, u32 w1)
{
    gsp_env[0] = w1 >> 24;
    gsp_env[1] = w1 >> 16;
    gsp_env[2] = w1 >>  8;
    gsp_env[3] = w1 >>  0;
}

/* 0xFC G_SETCOMBINE */
static void gsp_g_setcombine(u32 w0, u32 w1)
{
    u32 cc;
    u32 ac;
    if (gsp_cycle)
    {
        return;
    }
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
#ifdef GSP_FOG
    gsp_triangle =
        (gsp_geometry_mode & G_FOG) ? gsp_triangle_fog : gsp_triangle_default;
#else
    gsp_triangle = gsp_triangle_default;
#endif
#ifdef APP_UNSM
    gsp_texture_read_table[G_IM_FMT_IA << 2 | G_IM_SIZ_8b] =
        gsp_texture_read_ia8;
#endif
    switch (cc)
    {
    #ifdef APP_UNK4
        case CC1(0, 0, 0, 0):
            gsp_combine_cc = gsp_combine_cc_0;
            break;
    #endif
        case CC1(0, 0, 0, TEXEL0):
    #ifdef APP_UNSM
        case CC1(TEXEL1, TEXEL0, LOD_FRACTION, TEXEL0):
    #endif
            gsp_combine_cc = gsp_combine_cc_1;
            break;
        case CC1(0, 0, 0, SHADE):
        case CC1(TEXEL0, 0, SHADE, 0):
            gsp_combine_cc = gsp_combine_cc_shade;
            break;
    #ifdef APP_UNK4
        case CC1(0, 0, 0, PRIMITIVE):
    #endif
        case CC1(TEXEL0, 0, PRIMITIVE, 0):
    #ifdef APP_UNK4
        case CC1(PRIMITIVE, 0, TEXEL0, 0):
    #endif
            gsp_combine_cc = gsp_combine_cc_prim;
            break;
    #ifdef APP_UNK4
        case CC1(0, 0, 0, ENVIRONMENT):
    #endif
    #ifdef APP_UNSM
        case CC1(TEXEL0, 0, ENVIRONMENT, 0):
    #endif
            gsp_combine_cc = gsp_combine_cc_env;
            break;
    #ifdef APP_UNSM
        case CC1(SHADE, 0, ENVIRONMENT, 0):
            gsp_combine_cc = gsp_combine_cc_shade_env;
            break;
    #endif
    #ifdef APP_UNK4
        case CC1(PRIMITIVE, 0, SHADE, 0):
        case CC2(TEXEL0, 0, SHADE, 0, COMBINED, 0, PRIMITIVE, 0):
            gsp_combine_cc = gsp_combine_cc_shade_prim;
            break;
        case CC1(PRIMITIVE, 0, ENVIRONMENT, 0):
            gsp_combine_cc = gsp_combine_cc_prim_env;
            break;
        case CC1(PRIMITIVE, ENVIRONMENT, SHADE, ENVIRONMENT):
        case CC2(
            TEXEL0, 0, PRIMITIVE, 0,
            COMBINED, ENVIRONMENT, SHADE, ENVIRONMENT
        ):
            gsp_combine_cc = gsp_combine_cc_prim_env_shade_env;
            break;
    #endif
    #ifdef APP_UNSM
        case CC1(TEXEL0, SHADE, TEXEL0_ALPHA, SHADE):
            gsp_combine_cc = gsp_combine_cc_special1;
            gsp_triangle   = gsp_triangle_special;
            break;
        case CC1(PRIMITIVE, SHADE, TEXEL0, SHADE):
            gsp_combine_cc = gsp_combine_cc_special2;
            gsp_triangle   = gsp_triangle_special;
            gsp_texture_read_table[G_IM_FMT_IA << 2 | G_IM_SIZ_8b] =
                gsp_texture_read_ia8_special;
            break;
    #endif
    #ifdef APP_UNK4
        case CC1(SHADE, ENVIRONMENT, TEXEL0, ENVIRONMENT):
            gsp_combine_cc = gsp_combine_cc_special3;
            gsp_triangle   = gsp_triangle_special;
            break;
        case CC1(PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT):
            gsp_combine_cc = gsp_combine_cc_special4;
            gsp_triangle   = gsp_triangle_special;
            break;
        case CC2(
            PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, COMBINED, 0, SHADE, 0
        ):
            gsp_combine_cc = gsp_combine_cc_special5;
            gsp_triangle   = gsp_triangle_special;
            break;
    #endif
        default:
            gsp_combine_cc = gsp_combine_cc_0;
        #ifdef _DEBUG
            fprintf(
                stderr, "warning: unknown cc %08" FMT_X "%08" FMT_X "\n", w0, w1
            );
        #endif
            break;
    }
    switch (ac)
    {
    #ifdef APP_UNK4
        case AC1(0, 0, 0, 1):
    #endif
        case AC1(0, 0, 0, TEXEL0):
    #ifdef APP_UNK4
        case AC2(0, 0, 0, COMBINED, 0, 0, 0, 1):
        case AC1(TEXEL0, 0, 0, 1):
    #endif
    #ifdef APP_UNK4
        /* ??? */
        case AC1(PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT):
    #endif
            gsp_combine_ac = gsp_combine_ac_1;
            break;
        case AC1(0, 0, 0, SHADE):
        case AC1(TEXEL0, 0, SHADE, 0):
    #ifdef APP_UNK4
        case AC1(SHADE, 0, TEXEL0, 0):
    #endif
    #ifdef APP_UNSM
        case AC2(TEXEL1, TEXEL0, LOD_FRACTION, TEXEL0, 0, 0, 0, SHADE):
    #endif
            gsp_combine_ac = gsp_combine_ac_shade;
            break;
    #ifdef APP_UNK4
        case AC1(0, 0, 0, PRIMITIVE):
    #endif
        case AC1(TEXEL0, 0, PRIMITIVE, 0):
    #ifdef APP_UNK4
        case AC1(PRIMITIVE, 0, TEXEL0, 0):
    #endif
    #ifdef APP_UNSM
        case AC1(PRIMITIVE, SHADE, TEXEL0, SHADE):
    #endif
            gsp_combine_ac = gsp_combine_ac_prim;
            break;
    #ifdef APP_UNSM
        case AC1(0, 0, 0, ENVIRONMENT):
        case AC1(TEXEL0, 0, ENVIRONMENT, 0):
        case AC1(ENVIRONMENT, 0, TEXEL0, 0):
            gsp_combine_ac = gsp_combine_ac_env;
            break;
        case AC1(SHADE, 0, ENVIRONMENT, 0):
            gsp_combine_ac = gsp_combine_ac_shade_env;
            break;
    #endif
        default:
            gsp_combine_ac = gsp_combine_ac_0;
        #ifdef _DEBUG
            fprintf(
                stderr, "warning: unknown ac %08" FMT_X "%08" FMT_X "\n", w0, w1
            );
        #endif
            break;
    }
}

/* 0xFD G_SETTIMG */
static void gsp_g_settimg(unused u32 w0, u32 w1)
{
    gsp_texture_addr = gsp_addr(w1);
    gsp_change |= CHANGE_TEXTURE;
}

#ifdef _DEBUG
/* 0xFE G_SETZIMG */
static void gsp_g_setzimg(unused u32 w0, unused u32 w1)
{
}

/* 0xFF G_SETCIMG */
static void gsp_g_setcimg(unused u32 w0, unused u32 w1)
{
}
#endif

#ifdef GSP_F3DEX2
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

/*
static const char *const str_im_fmt[] = {"RGBA", "YUV", "CI", "IA", "I"};
*/

static void gsp_g_bg_1cyc(unused u32 w0, unused u32 w1)
{
    /*
    struct obj_bg_t *bg = gsp_addr(w1);
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

void gsp_init(void)
{
#ifdef GEKKO
    GX_ClearVtxDesc();
    GX_SetCurrentMtx(GX_PNMTX0);
    GX_SetVtxDesc(GX_VA_POS,  GX_INDEX16);
    GX_SetVtxDesc(GX_VA_TEX0, GX_INDEX16);
    GX_SetVtxDesc(GX_VA_CLR0, GX_INDEX16);
    GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS,  GX_POS_XYZ,  GX_S16,   0);
    GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST,   GX_F32,   0);
    GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
    GX_SetNumChans(1);
    GX_SetNumTexGens(1);
    GX_SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP_NULL, GX_COLOR0A0);
#else
#ifdef _3DS
    pglInit();
#endif
    glEnable(GL_SCISSOR_TEST);
    glEnable(GL_POLYGON_OFFSET_FILL);
#ifdef _3DS
    glDepthFunc(GL_LESS);
#endif
    glAlphaFunc(GL_GEQUAL, 0.25F);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
#ifndef GSP_LEGACY
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
#endif
#endif
#ifdef _3DS
    gsp_texture_buf = malloc(0x2000);
#else
#ifdef GEKKO
    gsp_texture_buf = memalign(0x20, 0x8000);
#else
    gsp_texture_buf = malloc(0x8000);
#endif
#endif
    gsp_cache_flag = true;
}

void gsp_destroy(void)
{
#ifdef _3DS
    pglExit();
#endif
}

void gsp_cache(void)
{
    gsp_cache_flag = true;
}

static void gsp_draw(void *ucode, u32 *dl)
{
#ifdef GEKKO
    GX_InvVtxCache();
    GX_InvalidateTexAll();
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
        gsp_dl_stack[gsp_dl_index] += 2;
        cmd = gsp_table[w0 >> 24];
        if (cmd != NULL)
        {
            cmd(w0, w1);
        }
        else
        {
        #ifdef _DEBUG
            fprintf(
                stderr,
                "error: invalid Gfx {{0x%08" FMT_X ", 0x%08" FMT_X "}}\n",
                w0, w1
            );
            exit(EXIT_FAILURE);
        #endif
        }
    }
    while (gsp_dl_index >= 0);
    gsp_flush_triangles();
#ifdef _3DS
    pglSwapBuffers();
#endif
#ifdef GEKKO
    GX_DrawDone();
#endif
}

void gsp_update(void *ucode, u32 *dl)
{
#ifdef _3DS
    f32 depth;
#endif
#ifdef APP_UNSM
    if (gsp_cache_flag)
#endif
    {
        gsp_cache_flag = false;
    #ifndef GEKKO
        if (gsp_texture > 0)
        {
            glDeleteTextures(gsp_texture, gsp_texture_name_table);
            gsp_texture = 0;
        }
    #endif
    }
#ifdef _3DS
    depth = gfxIsWide() ? 0 : (1.0F/3)*osGet3DSliderState();
    pglSelectScreen(GFX_TOP, GFX_LEFT);
    gsp_table[G_VTX] = gsp_g_vtx;
    gsp_write_triangle = gsp_write_triangle_init;
    gsp_depth = depth;
#endif
    gsp_draw(ucode, dl);
#ifdef _3DS
    if (depth > 0)
    {
        pglSwapBuffers();
        pglSelectScreen(GFX_TOP, GFX_RIGHT);
        gsp_table[G_VTX] = gsp_g_vtx_stub;
        gsp_write_triangle = gsp_write_triangle_stub;
        gsp_depth = -depth;
        gsp_draw(ucode, dl);
        pglSwapBuffers();
    }
#endif
}
#endif
