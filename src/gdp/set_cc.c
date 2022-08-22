static void gdp_combine_cc_0(u8 *col, UNUSED VTXF *vf)
{
    col[0] = 0x00;
    col[1] = 0x00;
    col[2] = 0x00;
}

static void gdp_combine_cc_1(u8 *col, UNUSED VTXF *vf)
{
    col[0] = 0xFF;
    col[1] = 0xFF;
    col[2] = 0xFF;
}

static void gdp_combine_cc_shade(u8 *col, VTXF *vf)
{
    col[0] = vf->shade[0];
    col[1] = vf->shade[1];
    col[2] = vf->shade[2];
}

static void gdp_combine_cc_prim(u8 *col, UNUSED VTXF *vf)
{
    col[0] = gdp_prim[0];
    col[1] = gdp_prim[1];
    col[2] = gdp_prim[2];
}

static void gdp_combine_cc_env(u8 *col, UNUSED VTXF *vf)
{
    col[0] = gdp_env[0];
    col[1] = gdp_env[1];
    col[2] = gdp_env[2];
}

static void gdp_combine_cc_shade_env(u8 *col, VTXF *vf)
{
    col[0] = vf->shade[0] * gdp_env[0] / 0x100;
    col[1] = vf->shade[1] * gdp_env[1] / 0x100;
    col[2] = vf->shade[2] * gdp_env[2] / 0x100;
}

#ifndef APP_UNSM
static void gdp_combine_cc_shade_prim(u8 *col, VTXF *vf)
{
    col[0] = vf->shade[0] * gdp_prim[0] / 0x100;
    col[1] = vf->shade[1] * gdp_prim[1] / 0x100;
    col[2] = vf->shade[2] * gdp_prim[2] / 0x100;
}

static void gdp_combine_cc_prim_env(u8 *col, UNUSED VTXF *vf)
{
    col[0] = gdp_prim[0] * gdp_env[0] / 0x100;
    col[1] = gdp_prim[1] * gdp_env[1] / 0x100;
    col[2] = gdp_prim[2] * gdp_env[2] / 0x100;
}

static void gdp_combine_cc_prim_env_shade_env(u8 *col, VTXF *vf)
{
    col[0] = (gdp_prim[0]-gdp_env[0]) * vf->shade[0]/0x100 + gdp_env[0];
    col[1] = (gdp_prim[1]-gdp_env[1]) * vf->shade[1]/0x100 + gdp_env[1];
    col[2] = (gdp_prim[2]-gdp_env[2]) * vf->shade[2]/0x100 + gdp_env[2];
}

static void gdp_combine_cc_1env(u8 *col, UNUSED VTXF *vf)
{
    col[0] = 0xFF - gdp_env[0];
    col[1] = 0xFF - gdp_env[1];
    col[2] = 0xFF - gdp_env[2];
}
#endif

#ifdef GSP_SWFOG
static void gdp_combine_cc_fog(u8 *col, UNUSED VTXF *vf)
{
    col[0] = gdp_fog[0];
    col[1] = gdp_fog[1];
    col[2] = gdp_fog[2];
}
#endif

static void gdp_combine_ac_0(u8 *col, UNUSED VTXF *vf)
{
    col[3] = 0x00;
}

static void gdp_combine_ac_1(u8 *col, UNUSED VTXF *vf)
{
    col[3] = 0xFF;
}

static void gdp_combine_ac_shade(u8 *col, VTXF *vf)
{
    col[3] = vf->shade[3];
}

static void gdp_combine_ac_prim(u8 *col, UNUSED VTXF *vf)
{
    col[3] = gdp_prim[3];
}

static void gdp_combine_ac_env(u8 *col, UNUSED VTXF *vf)
{
    col[3] = gdp_env[3];
}

#ifndef APP_UNSM
static void gdp_combine_ac_shade_prim(u8 *col, UNUSED VTXF *vf)
{
    col[3] = vf->shade[3] * gdp_prim[3] / 0x100;
}
#endif

static void gdp_combine_ac_shade_env(u8 *col, UNUSED VTXF *vf)
{
    col[3] = vf->shade[3] * gdp_env[3] / 0x100;
}

#ifdef GSP_SWFOG
static void gdp_combine_ac_fog(u8 *col, VTXF *vf)
{
    col[3] = vf->shade[3];
}
#endif

static void gdp_set_tx(GDP_TX tx);

static void gdp_tri(const u8 *t);
#ifdef GSP_SWFOG
static void gdp_tri_fog(const u8 *t);
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
        case CC1(1, 0, PRIMITIVE, 0):
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
        case CC2(TEXEL0, 0, SHADE, 0, ENVIRONMENT, 0, COMBINED, 0):
            gdp_combine_cc = gdp_combine_cc_shade_env;
            tx = GDP_TX_MODULATE;
            break;
        case CC1(SHADE, 0, PRIMITIVE, 0):
        case CC1(PRIMITIVE, 0, SHADE, 0):
            gdp_combine_cc = gdp_combine_cc_shade_prim;
            tx = GDP_TX_NULL;
            break;
        case CC2(TEXEL0, 0, SHADE, 0, COMBINED, 0, PRIMITIVE, 0):
        case CC2(TEXEL0, 0, SHADE, 0, PRIMITIVE, 0, COMBINED, 0):
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
        case CC1(TEXEL1, TEXEL0, PRIMITIVE_ALPHA, TEXEL0): /*this is wrong*/
        case CC2(
            TEXEL0, PRIMITIVE, ENV_ALPHA, TEXEL0,
            PRIMITIVE, ENVIRONMENT, COMBINED, ENVIRONMENT
        ): /*this is wrong*/
        case CC2(
            TEXEL1, PRIMITIVE, ENV_ALPHA, TEXEL0,
            PRIMITIVE, ENVIRONMENT, COMBINED, ENVIRONMENT
        ): /*this is wrong*/
        case CC2(
            TEXEL1, PRIMITIVE, PRIM_LOD_FRAC, TEXEL0,
            PRIMITIVE, ENVIRONMENT, COMBINED, ENVIRONMENT
        ): /*this is wrong*/
            gdp_combine_cc = gdp_combine_cc_1;
            tx = GDP_TX_MODULATE;
            break;
        case CC2(
            TEXEL1, TEXEL0, ENV_ALPHA, TEXEL0,
            COMBINED, 0, SHADE, 0
        ): /*this is wrong*/
            gdp_combine_cc = gdp_combine_cc_shade;
            tx = GDP_TX_MODULATE;
            break;
        case CC2(
            TEXEL1, TEXEL0, ENV_ALPHA, TEXEL0,
            PRIMITIVE, ENVIRONMENT, COMBINED, ENVIRONMENT
        ): /*this is wrong*/
            gdp_combine_cc = gdp_combine_cc_prim;
            tx = GDP_TX_MODULATE;
            break;
#endif
        default:
            gdp_combine_cc = gdp_combine_cc_1;
            tx = GDP_TX_NULL;
            /* wdebug("unknown cc %08" FMT_X "%08" FMT_X "\n", w0, w1); */
            break;
    }
    switch (ac)
    {
#ifndef APP_UNSM
        case AC1(0, 0, 0, 1):
#endif
        case AC1(0, 0, 0, TEXEL0):
#ifndef APP_UNSM
        case AC2(0, 0, 0, 0, 0, 0, 0, 1):
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
        case AC2(0, 0, 0, TEXEL0, COMBINED, 0, PRIMITIVE, 0):
#endif
        case AC1(PRIMITIVE, SHADE, TEXEL0, SHADE): /*this is wrong*/
#ifndef APP_UNSM
        case AC1(PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT): /*this is wrong*/
        case AC2(0, 0, 0, 1, COMBINED, 0, PRIMITIVE, 0):
        case AC2(
            TEXEL1, 1, PRIM_LOD_FRAC, TEXEL0,
            COMBINED, 0, PRIMITIVE, 0
        ): /*this is wrong*/
        case AC2(
            TEXEL0, 0, PRIMITIVE, 0,
            TEXEL1, 0, PRIM_LOD_FRAC, COMBINED
        ): /*this is wrong*/
#endif
            gdp_combine_ac = gdp_combine_ac_prim;
            break;
        case AC1(0, 0, 0, ENVIRONMENT):
        case AC1(TEXEL0, 0, ENVIRONMENT, 0):
        case AC1(ENVIRONMENT, 0, TEXEL0, 0):
            gdp_combine_ac = gdp_combine_ac_env;
            break;
#ifndef APP_UNSM
        case AC2(TEXEL0, 0, SHADE, 0, COMBINED, 0, PRIMITIVE, 0):
            gdp_combine_ac = gdp_combine_ac_shade_prim;
            break;
#endif
        case AC1(SHADE, 0, ENVIRONMENT, 0):
            gdp_combine_ac = gdp_combine_ac_shade_env;
            break;
#ifndef APP_UNSM
        case AC1(TEXEL1, TEXEL0, PRIMITIVE, TEXEL0): /*this is wrong*/
        case AC1(TEXEL1, TEXEL0, PRIM_LOD_FRAC, TEXEL0): /*this is wrong*/
            gdp_combine_ac = gdp_combine_ac_1;
            break;
        case AC2(
            TEXEL1, TEXEL0, ENVIRONMENT, TEXEL0,
            COMBINED, 0, PRIMITIVE, 0
        ): /*this is wrong*/
            gdp_combine_ac = gdp_combine_ac_prim;
            break;
#endif
        default:
            gdp_combine_ac = gdp_combine_ac_1;
            /* wdebug("unknown ac %08" FMT_X "%08" FMT_X "\n", w0, w1); */
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
