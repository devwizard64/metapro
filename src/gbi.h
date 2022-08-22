#ifndef __GBI_H__
#define __GBI_H__

#define G_OFF       0x00
#define G_ON        0x01

#ifdef GSP_F3D
#define G_SPNOOP                0x00
#define G_MTX                   0x01
#define G_MOVEMEM               0x03
#define G_VTX                   0x04
#define G_DL                    0x06
#ifdef GSP_F3D_20D
#define G_RDPHALF_CONT          0xB1
#define G_RDPHALF_2             0xB2
#define G_RDPHALF_1             0xB3
#define G_PERSPNORMALIZE        0xB4
#else
#ifdef GSP_F3DEX
#define G_LOAD_UCODE            0xAF
#define G_BRANCH_Z              0xB0
#define G_TRI2                  0xB1
#define G_MODIFYVTX             0xB2
#else
#define G_RDPHALF_CONT          0xB2
#endif
#define G_RDPHALF_2             0xB3
#define G_RDPHALF_1             0xB4
#ifdef GSP_F3DEX
#define G_QUAD                  0xB5
#endif
#endif
#define G_CLEARGEOMETRYMODE     0xB6
#define G_SETGEOMETRYMODE       0xB7
#define G_ENDDL                 0xB8
#define G_SETOTHERMODE_L        0xB9
#define G_SETOTHERMODE_H        0xBA
#define G_TEXTURE               0xBB
#define G_MOVEWORD              0xBC
#define G_POPMTX                0xBD
#define G_CULLDL                0xBE
#define G_TRI1                  0xBF
#define G_NOOP                  0xC0
#endif
#ifdef GSP_F3DEX2
#define G_SPNOOP                0x00
#define G_VTX                   0x01
#define G_MODIFYVTX             0x02
#define G_CULLDL                0x03
#define G_BRANCH_Z              0x04
#define G_TRI1                  0x05
#define G_TRI2                  0x06
#define G_LINE3D                0x07
#define G_DMA_IO                0xD6
#define G_TEXTURE               0xD7
#define G_POPMTX                0xD8
#define G_GEOMETRYMODE          0xD9
#define G_MTX                   0xDA
#define G_MOVEWORD              0xDB
#define G_MOVEMEM               0xDC
#define G_LOAD_UCODE            0xDD
#define G_DL                    0xDE
#define G_ENDDL                 0xDF
#define G_NOOP                  0xE0
#define G_RDPHALF_1             0xE1
#define G_SETOTHERMODE_L        0xE2
#define G_SETOTHERMODE_H        0xE3

#define G_OBJ_RECTANGLE         0x01
#define G_OBJ_SPRITE            0x02
#define G_SELECT_DL             0x04
#define G_OBJ_LOADTXTR          0x05
#define G_OBJ_LDTX_SPRITE       0x06
#define G_OBJ_LDTX_RECT         0x07
#define G_OBJ_LDTX_RECT_R       0x08
#define G_BG_1CYC               0x09
#define G_BG_COPY               0x0A
#define G_OBJ_RENDERMODE        0x0B
#define G_OBJ_RECTANGLE_R       0xDA
#define G_OBJ_MOVEMEM           0xDC
#define G_RDPHALF_0             0xE4
#endif
#define G_TEXRECT               0xE4
#define G_TEXRECTFLIP           0xE5
#define G_RDPLOADSYNC           0xE6
#define G_RDPPIPESYNC           0xE7
#define G_RDPTILESYNC           0xE8
#define G_RDPFULLSYNC           0xE9
#define G_SETKEYGB              0xEA
#define G_SETKEYR               0xEB
#define G_SETCONVERT            0xEC
#define G_SETSCISSOR            0xED
#define G_SETPRIMDEPTH          0xEE
#define G_RDPSETOTHERMODE       0xEF
#define G_LOADTLUT              0xF0
#ifdef GSP_F3DEX2
#define G_RDPHALF_2             0xF1
#endif
#define G_SETTILESIZE           0xF2
#define G_LOADBLOCK             0xF3
#define G_LOADTILE              0xF4
#define G_SETTILE               0xF5
#define G_FILLRECT              0xF6
#define G_SETFILLCOLOR          0xF7
#define G_SETFOGCOLOR           0xF8
#define G_SETBLENDCOLOR         0xF9
#define G_SETPRIMCOLOR          0xFA
#define G_SETENVCOLOR           0xFB
#define G_SETCOMBINE            0xFC
#define G_SETTIMG               0xFD
#define G_SETZIMG               0xFE
#define G_SETCIMG               0xFF

#ifdef GSP_F3D
#define G_MTX_MODELVIEW         0x00
#define G_MTX_PROJECTION        0x01
#define G_MTX_MUL               0x00
#define G_MTX_LOAD              0x02
#define G_MTX_NOPUSH            0x00
#define G_MTX_PUSH              0x04
#endif

#ifdef GSP_F3DEX2
#define G_MTX_PUSH              0x00
#define G_MTX_NOPUSH            0x01
#define G_MTX_MUL               0x00
#define G_MTX_LOAD              0x02
#define G_MTX_MODELVIEW         0x00
#define G_MTX_PROJECTION        0x04
#endif

#define G_ZBUFFER               0x00000001
#define G_TEXTURE_ENABLE        0x00000002
#define G_SHADE                 0x00000004
#ifdef GSP_F3DEX2
#define G_CULL_FRONT            0x00000200
#define G_CULL_BACK             0x00000400
#define G_CULL_BOTH             0x00000600
#define G_SHADING_SMOOTH        0x00200000
#else
#define G_SHADING_SMOOTH        0x00000200
#define G_CULL_FRONT            0x00001000
#define G_CULL_BACK             0x00002000
#define G_CULL_BOTH             0x00003000
#endif
#define G_FOG                   0x00010000
#define G_LIGHTING              0x00020000
#define G_TEXTURE_GEN           0x00040000
#define G_TEXTURE_GEN_LINEAR    0x00080000
#define G_LOD                   0x00100000

#define G_IM_FMT_RGBA   0x00
#define G_IM_FMT_YUV    0x01
#define G_IM_FMT_CI     0x02
#define G_IM_FMT_IA     0x03
#define G_IM_FMT_I      0x04

#define G_IM_SIZ_4b     0x00
#define G_IM_SIZ_8b     0x01
#define G_IM_SIZ_16b    0x02
#define G_IM_SIZ_32b    0x03

#define G_CCMUX_COMBINED            0x00
#define G_CCMUX_TEXEL0              0x01
#define G_CCMUX_TEXEL1              0x02
#define G_CCMUX_PRIMITIVE           0x03
#define G_CCMUX_SHADE               0x04
#define G_CCMUX_ENVIRONMENT         0x05
#define G_CCMUX_CENTER              0x06
#define G_CCMUX_SCALE               0x06
#define G_CCMUX_COMBINED_ALPHA      0x07
#define G_CCMUX_TEXEL0_ALPHA        0x08
#define G_CCMUX_TEXEL1_ALPHA        0x09
#define G_CCMUX_PRIMITIVE_ALPHA     0x0A
#define G_CCMUX_SHADE_ALPHA         0x0B
#define G_CCMUX_ENV_ALPHA           0x0C
#define G_CCMUX_LOD_FRACTION        0x0D
#define G_CCMUX_PRIM_LOD_FRAC       0x0E
#define G_CCMUX_NOISE               0x07
#define G_CCMUX_K4                  0x07
#define G_CCMUX_K5                  0x0F
#define G_CCMUX_1                   0x06
#define G_CCMUX_0                   0x1F

#define G_ACMUX_COMBINED        0x00
#define G_ACMUX_TEXEL0          0x01
#define G_ACMUX_TEXEL1          0x02
#define G_ACMUX_PRIMITIVE       0x03
#define G_ACMUX_SHADE           0x04
#define G_ACMUX_ENVIRONMENT     0x05
#define G_ACMUX_LOD_FRACTION    0x00
#define G_ACMUX_PRIM_LOD_FRAC   0x06
#define G_ACMUX_1               0x06
#define G_ACMUX_0               0x07

#define G_MDSFT_ALPHACOMPARE    0x00
#define G_MDSFT_ZSRCSEL         0x02
#define G_MDSFT_RENDERMODE      0x03
#define G_MDSFT_BLENDER         0x10

#define G_MDSFT_BLENDMASK       0x00
#define G_MDSFT_ALPHADITHER     0x04
#define G_MDSFT_RGBDITHER       0x06
#define G_MDSFT_COMBKEY         0x08
#define G_MDSFT_TEXTCONV        0x09
#define G_MDSFT_TEXTFILT        0x0C
#define G_MDSFT_TEXTLUT         0x0E
#define G_MDSFT_TEXTLOD         0x10
#define G_MDSFT_TEXTDETAIL      0x11
#define G_MDSFT_TEXTPERSP       0x13
#define G_MDSFT_CYCLETYPE       0x14
#define G_MDSFT_COLORDITHER     0x16
#define G_MDSFT_PIPELINE        0x17

#define G_PM_NPRIMITIVE (0x00 << G_MDSFT_PIPELINE)
#define G_PM_1PRIMITIVE (0x01 << G_MDSFT_PIPELINE)

#define G_CYC_1CYCLE    (0x00 << G_MDSFT_CYCLETYPE)
#define G_CYC_2CYCLE    (0x01 << G_MDSFT_CYCLETYPE)
#define G_CYC_COPY      (0x02 << G_MDSFT_CYCLETYPE)
#define G_CYC_FILL      (0x03 << G_MDSFT_CYCLETYPE)

#define G_TP_NONE       (0x00 << G_MDSFT_TEXTPERSP)
#define G_TP_PERSP      (0x01 << G_MDSFT_TEXTPERSP)

#define G_TD_CLAMP      (0x00 << G_MDSFT_TEXTDETAIL)
#define G_TD_SHARPEN    (0x01 << G_MDSFT_TEXTDETAIL)
#define G_TD_DETAIL     (0x02 << G_MDSFT_TEXTDETAIL)

#define G_TL_TILE       (0x00 << G_MDSFT_TEXTLOD)
#define G_TL_LOD        (0x01 << G_MDSFT_TEXTLOD)

#define G_TT_NONE       (0x00 << G_MDSFT_TEXTLUT)
#define G_TT_RGBA16     (0x02 << G_MDSFT_TEXTLUT)
#define G_TT_IA16       (0x03 << G_MDSFT_TEXTLUT)

#define G_TF_POINT      (0x00 << G_MDSFT_TEXTFILT)
#define G_TF_AVERAGE    (0x03 << G_MDSFT_TEXTFILT)
#define G_TF_BILERP     (0x02 << G_MDSFT_TEXTFILT)

#define G_TC_CONV       (0x00 << G_MDSFT_TEXTCONV)
#define G_TC_FILTCONV   (0x05 << G_MDSFT_TEXTCONV)
#define G_TC_FILT       (0x06 << G_MDSFT_TEXTCONV)

#define G_CK_NONE       (0x00 << G_MDSFT_COMBKEY)
#define G_CK_KEY        (0x01 << G_MDSFT_COMBKEY)

#define G_CD_MAGICSQ    (0x00 << G_MDSFT_RGBDITHER)
#define G_CD_BAYER      (0x01 << G_MDSFT_RGBDITHER)
#define G_CD_NOISE      (0x02 << G_MDSFT_RGBDITHER)

#define G_CD_DISABLE    (0x03 << G_MDSFT_RGBDITHER)
#define G_CD_ENABLE     G_CD_NOISE

#define G_AD_PATTERN    (0x00 << G_MDSFT_ALPHADITHER)
#define G_AD_NOTPATTERN (0x01 << G_MDSFT_ALPHADITHER)
#define G_AD_NOISE      (0x02 << G_MDSFT_ALPHADITHER)
#define G_AD_DISABLE    (0x03 << G_MDSFT_ALPHADITHER)

#define G_AC_NONE       (0x00 << G_MDSFT_ALPHACOMPARE)
#define G_AC_THRESHOLD  (0x01 << G_MDSFT_ALPHACOMPARE)
#define G_AC_DITHER     (0x03 << G_MDSFT_ALPHACOMPARE)

#define G_ZS_PIXEL      (0x00 << G_MDSFT_ZSRCSEL)
#define G_ZS_PRIM       (0x01 << G_MDSFT_ZSRCSEL)

#define AA_EN           0x0008
#define Z_CMP           0x0010
#define Z_UPD           0x0020
#define IM_RD           0x0040
#define CLR_ON_CVG      0x0080
#define CVG_DST_CLAMP   0x0000
#define CVG_DST_WRAP    0x0100
#define CVG_DST_FULL    0x0200
#define CVG_DST_SAVE    0x0300
#define ZMODE_OPA       0x0000
#define ZMODE_INTER     0x0400
#define ZMODE_XLU       0x0800
#define ZMODE_DEC       0x0C00
#define CVG_X_ALPHA     0x1000
#define ALPHA_CVG_SEL   0x2000
#define FORCE_BL        0x4000

#define G_RM_AA_ZB_XLU_DECAL    0x00404DD8
#define G_RM_AA_ZB_XLU_DECAL2   0x00104DD8
#define G_RM_OPA_SURF           0x0C084000
#define G_RM_OPA_SURF2          0x03024000

#define G_DL_PUSH   0x00
#define G_DL_NOPUSH 0x01

#ifdef GSP_F3D
#define G_MV_VIEWPORT   0x80
#define G_MV_LOOKATY    0x82
#define G_MV_LOOKATX    0x84
#define G_MV_L0         0x86
#define G_MV_L1         0x88
#define G_MV_L2         0x8A
#define G_MV_L3         0x8C
#define G_MV_L4         0x8E
#define G_MV_L5         0x90
#define G_MV_L6         0x92
#define G_MV_L7         0x94
#define G_MV_TXTATT     0x96
#define G_MV_MATRIX_2   0x98
#define G_MV_MATRIX_3   0x9A
#define G_MV_MATRIX_4   0x9C
#define G_MV_MATRIX_1   0x9E
#endif
#ifdef GSP_F3DEX2
#define G_MV_VIEWPORT   0x08
#define G_MV_LIGHT      0x0A
#define G_MV_POINT      0x0C
#define G_MV_MATRIX     0x0E
#define G_MVO_LOOKATX   (3*0)
#define G_MVO_LOOKATY   (3*1)
#define G_MVO_L0        (3*2)
#define G_MVO_L1        (3*3)
#define G_MVO_L2        (3*4)
#define G_MVO_L3        (3*5)
#define G_MVO_L4        (3*6)
#define G_MVO_L5        (3*7)
#define G_MVO_L6        (3*8)
#define G_MVO_L7        (3*9)
#endif

#define G_MW_MATRIX     0x00
#define G_MW_NUMLIGHT   0x02
#define G_MW_CLIP       0x04
#define G_MW_SEGMENT    0x06
#define G_MW_FOG        0x08
#define G_MW_LIGHTCOL   0x0A
#ifdef GSP_F3DEX
#define G_MW_FORCEMTX   0x0C
#else
#define G_MW_POINTS     0x0C
#endif
#ifndef GSP_F3D_20D
#define G_MW_PERSPNORM  0x0E
#endif

#define G_MWO_POINT_RGBA        0x10
#define G_MWO_POINT_ST          0x14
#define G_MWO_POINT_XYSCREEN    0x18
#define G_MWO_POINT_ZSCREEN     0x1C

#define G_TX_RENDERTILE 0x00
#define G_TX_LOADTILE   0x07

#define G_TX_NOMIRROR   0
#define G_TX_WRAP       0
#define G_TX_MIRROR     1
#define G_TX_CLAMP      2
#define G_TX_NOMASK     0
#define G_TX_NOLOD      0

#ifdef GSP_F3DEX2
#define G_BGLT_LOADBLOCK        0x0033
#define G_BGLT_LOADTILE         0xFFF4
#define G_BG_FLAG_FLIPS         0x01
#define G_BG_FLAG_FLIPT         0x10
#define G_OBJ_FLAG_FLIPS        0x01
#define G_OBJ_FLAG_FLIPT        0x10
#define G_OBJLT_TXTRBLOCK       0x00001033
#define G_OBJLT_TXTRTILE        0x00FC1034
#define G_OBJLT_TLUT            0x00000030
#define G_MW_GENSTAT            0x08
#define G_OBJRM_NOTXCLAMP       0x01
#define G_OBJRM_XLU             0x02
#define G_OBJRM_ANTIALIAS       0x04
#define G_OBJRM_BILERP          0x08
#define G_OBJRM_SHRINKSIZE_1    0x10
#define G_OBJRM_SHRINKSIZE_2    0x20
#define G_OBJRM_WIDEN           0x40
#endif

#ifndef __ASSEMBLER__

typedef struct vp
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
}
VP;

typedef struct vtx
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
}
VTX;

typedef struct light
{
#ifdef __EB__
    struct
    {
        u8 r;
        u8 g;
        u8 b;
        u8 f;
    }
    col[2];
    s8 x;
    s8 y;
    s8 z;
    u8 f;
#else
    struct
    {
        u8 f;
        u8 b;
        u8 g;
        u8 r;
    }
    col[2];
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
}
LIGHT;

typedef struct obj_bg
{
#ifdef __EB__
    u16 image_x;
    u16 image_w;
    s16 frame_x;
    u16 frame_w;
    u16 image_y;
    u16 image_h;
    s16 frame_y;
    u16 frame_h;
    PTR image_ptr;
    u16 image_load;
    u8  image_fmt;
    u8  image_siz;
    u16 image_pal;
    u16 image_flip;
    u16 scale_w;
    u16 scale_h;
    s32 image_y_orig;
    u32 pad;
#else
    u16 image_w;
    u16 image_x;
    u16 frame_w;
    s16 frame_x;
    u16 image_h;
    u16 image_y;
    u16 frame_h;
    s16 frame_y;
    PTR image_ptr;
    u8  image_siz;
    u8  image_fmt;
    u16 image_load;
    u16 image_flip;
    u16 image_pal;
    u16 scale_h;
    u16 scale_w;
    s32 image_y_orig;
    u32 pad;
#endif
}
uObjBg;

typedef struct obj_sprite
{
#ifdef __EB__
    s16 obj_x;
    u16 scale_w;
    u16 image_w;
    u16 pad_x;
    s16 obj_y;
    u16 scale_h;
    u16 image_h;
    u16 pad_y;
    u16 image_stride;
    u16 image_adrs;
    u8  image_fmt;
    u8  image_siz;
    u8  image_pal;
    u8  image_flag;
#else
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
#endif
}
uObjSprite;

typedef struct obj_mtx
{
#ifdef __EB__
    s32 a;
    s32 b;
    s32 c;
    s32 d;
    s16 x;
    s16 y;
    u16 base_scale_x;
    u16 base_scale_y;
#else
    s32 a;
    s32 b;
    s32 c;
    s32 d;
    s16 y;
    s16 x;
    u16 base_scale_y;
    u16 base_scale_x;
#endif
}
uObjMtx;

typedef struct obj_sub_mtx
{
#ifdef __EB__
    s16 x;
    s16 y;
    u16 base_scale_x;
    u16 base_scale_y;
#else
    s16 y;
    s16 x;
    u16 base_scale_y;
    u16 base_scale_x;
#endif
}
uObjSubMtx;

typedef struct obj_txtr
{
#ifdef __EB__
    u32 type;
    PTR image;
    u16 tmem; /* phead for tlut */
    u16 twidth; /* tsize for block, pnum for tlut */
    u16 theight; /* tline for block */
    u16 sid;
    u32 flag;
    u32 mask;
#else
    u32 type;
    PTR image;
    u16 twidth;
    u16 tmem;
    u16 sid;
    u16 theight;
    u32 flag;
    u32 mask;
#endif
}
uObjTxtr;

#endif /* __ASSEMBLER__ */

#endif /* __GBI_H__ */
