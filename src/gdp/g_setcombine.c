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
#endif

#ifdef GSP_FOG
static void gdp_combine_cc_fog(u8 *col, unused struct vtxf *vf)
{
    col[0] = gdp_fog[0];
    col[1] = gdp_fog[1];
    col[2] = gdp_fog[2];
}
#endif

static void gdp_combine_cc_special1(u8 *col, struct vtxf *vf)
{
    if (gsp_texture_enabled)
    {
        col[0] = 0xFF;
        col[1] = 0xFF;
        col[2] = 0xFF;
    }
    else
    {
        col[0] = vf->shade[0];
        col[1] = vf->shade[1];
        col[2] = vf->shade[2];
    }
}

#ifdef APP_UNSM
static void gdp_combine_cc_special2(u8 *col, struct vtxf *vf)
{
    if (gsp_texture_enabled)
    {
        col[0] = gdp_prim[0];
        col[1] = gdp_prim[1];
        col[2] = gdp_prim[2];
    }
    else
    {
        col[0] = vf->shade[0];
        col[1] = vf->shade[1];
        col[2] = vf->shade[2];
    }
}
#endif

#ifdef APP_UNK4
static void gdp_combine_cc_special3(u8 *col, struct vtxf *vf)
{
    if (gsp_texture_enabled)
    {
        col[0] = vf->shade[0];
        col[1] = vf->shade[1];
        col[2] = vf->shade[2];
    }
    else
    {
        col[0] = gdp_env[0];
        col[1] = gdp_env[1];
        col[2] = gdp_env[2];
    }
}
#endif

static void gdp_combine_cc_special4(u8 *col, unused struct vtxf *vf)
{
    if (gsp_texture_enabled)
    {
        col[0] = gdp_prim[0];
        col[1] = gdp_prim[1];
        col[2] = gdp_prim[2];
    }
    else
    {
        col[0] = gdp_env[0];
        col[1] = gdp_env[1];
        col[2] = gdp_env[2];
    }
}

#ifdef APP_UNK4
static void gdp_combine_cc_special5(u8 *col, struct vtxf *vf)
{
    gdp_combine_cc_special4(col, vf);
    col[0] = col[0] * vf->shade[0] / 0x100;
    col[1] = col[1] * vf->shade[1] / 0x100;
    col[2] = col[2] * vf->shade[2] / 0x100;
}
#endif

#ifdef APP_UNKT
static void gdp_combine_cc_special6(u8 *col, unused struct vtxf *vf)
{
    if (gsp_texture_enabled)
    {
        col[0] = 0xFF - gdp_env[0];
        col[1] = 0xFF - gdp_env[1];
        col[2] = 0xFF - gdp_env[2];
    }
    else
    {
        col[0] = gdp_prim[0];
        col[1] = gdp_prim[1];
        col[2] = gdp_prim[2];
    }
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

#ifdef GSP_FOG
static void gdp_combine_ac_fog(u8 *col, struct vtxf *vf)
{
    col[3] = vf->shade[3];
}
#endif

static void gdp_triangle_default(u8 *t)
{
#ifndef GSP_LEGACY
    if (gdp_output_total < GDP_OUTPUT_LEN)
#endif
    {
        gdp_write_triangle(t);
    }
#ifndef GSP_LEGACY
    else
    {
        wdebug("output over\n");
    }
#endif
}

#ifdef GSP_FOG
static void gdp_triangle_fog(u8 *t)
{
#ifndef GSP_LEGACY
    if (gdp_output_total < GDP_OUTPUT_LEN-1)
#endif
    {
        u8 sa0;
        u8 sa1;
        u8 sa2;
        gdp_write_triangle(t);
        sa0 = gsp_vtxf_buf[t[0]].shade[3];
        sa1 = gsp_vtxf_buf[t[1]].shade[3];
        sa2 = gsp_vtxf_buf[t[2]].shade[3];
        if (sa0 > 0x00 || sa1 > 0x00 || sa2 > 0x00)
        {
            u32 othermode_l;
            u8  texture_enabled;
            GDP_COMBINE *combine_cc;
            GDP_COMBINE *combine_ac;
            othermode_l = gdp_othermode_l;
            gsp_g_setothermode_l(
                G_SETOTHERMODE_L << 24 | 3 << 8 | 29,
                G_RM_AA_ZB_XLU_DECAL | G_RM_AA_ZB_XLU_DECAL2
            );
            texture_enabled = gsp_texture_enabled;
            combine_cc = gdp_combine_cc;
            combine_ac = gdp_combine_ac;
            gsp_texture_enabled = G_OFF;
            gdp_combine_cc = gdp_combine_cc_fog;
            gdp_combine_ac = gdp_combine_ac_fog;
            gsp_change |= CHANGE_TEXTURE_ENABLED;
            gdp_write_triangle(t);
            gdp_othermode_l = othermode_l;
            gsp_texture_enabled = texture_enabled;
            gdp_combine_cc = combine_cc;
            gdp_combine_ac = combine_ac;
            gsp_change |= CHANGE_RENDERMODE | CHANGE_TEXTURE_ENABLED;
        }
    }
#ifndef GSP_LEGACY
    else
    {
        wdebug("output over\n");
    }
#endif
}
#endif

static void gdp_triangle_special(u8 *t)
{
#ifndef GSP_LEGACY
    if (gdp_output_total < GDP_OUTPUT_LEN-1)
#endif
    {
        u8  texture_enabled;
        u32 othermode_l;
        texture_enabled = gsp_texture_enabled;
        gsp_texture_enabled = G_OFF;
        gsp_change |= CHANGE_TEXTURE_ENABLED;
        gdp_write_triangle(t);
        othermode_l = gdp_othermode_l;
        gsp_g_setothermode_l(
            G_SETOTHERMODE_L << 24 | 3 << 8 | 29,
            G_RM_AA_ZB_XLU_DECAL | G_RM_AA_ZB_XLU_DECAL2
        );
        gsp_texture_enabled = texture_enabled;
        gsp_change |= CHANGE_TEXTURE_ENABLED;
        gdp_write_triangle(t);
        gdp_othermode_l = othermode_l;
        gsp_change |= CHANGE_RENDERMODE;
    }
#ifndef GSP_LEGACY
    else
    {
        wdebug("warning: output over\n");
    }
#endif
}

static void gdp_g_setcombine(u32 w0, u32 w1)
{
    u32 cc;
    u32 ac;
    if (gdp_cycle)
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
    gdp_triangle =
        (gsp_geometry_mode & G_FOG) ? gdp_triangle_fog : gdp_triangle_default;
#else
    gdp_triangle = gdp_triangle_default;
#endif
#ifdef APP_UNSM
    gdp_texture_read_table[G_IM_FMT_IA << 2 | G_IM_SIZ_8b] =
        gdp_texture_read_ia8;
#endif
    switch (cc)
    {
    #ifndef APP_UNSM
        case CC1(0, 0, 0, 0):
            gdp_combine_cc = gdp_combine_cc_0;
            break;
        case CC1(0, 0, 0, 1):
            gdp_combine_cc = gdp_combine_cc_1;
            break;
    #endif
        case CC1(0, 0, 0, TEXEL0):
        case CC1(TEXEL1, TEXEL0, LOD_FRACTION, TEXEL0):
            gdp_combine_cc = gdp_combine_cc_1;
            break;
        case CC1(0, 0, 0, SHADE):
        case CC1(TEXEL0, 0, SHADE, 0):
            gdp_combine_cc = gdp_combine_cc_shade;
            break;
    #ifndef APP_UNSM
        case CC1(0, 0, 0, PRIMITIVE):
    #endif
        case CC1(TEXEL0, 0, PRIMITIVE, 0):
    #ifndef APP_UNSM
        case CC1(PRIMITIVE, 0, TEXEL0, 0):
    #endif
            gdp_combine_cc = gdp_combine_cc_prim;
            break;
    #ifndef APP_UNSM
        case CC1(0, 0, 0, ENVIRONMENT):
    #endif
        case CC1(TEXEL0, 0, ENVIRONMENT, 0):
            gdp_combine_cc = gdp_combine_cc_env;
            break;
        case CC1(SHADE, 0, ENVIRONMENT, 0):
            gdp_combine_cc = gdp_combine_cc_shade_env;
            break;
    #ifndef APP_UNSM
        case CC1(PRIMITIVE, 0, SHADE, 0):
        case CC2(TEXEL0, 0, SHADE, 0, COMBINED, 0, PRIMITIVE, 0):
            gdp_combine_cc = gdp_combine_cc_shade_prim;
            break;
        case CC1(PRIMITIVE, 0, ENVIRONMENT, 0):
            gdp_combine_cc = gdp_combine_cc_prim_env;
            break;
        case CC1(PRIMITIVE, ENVIRONMENT, SHADE, ENVIRONMENT):
        case CC2(
            TEXEL0, 0, PRIMITIVE, 0,
            COMBINED, ENVIRONMENT, SHADE, ENVIRONMENT
        ):
            gdp_combine_cc = gdp_combine_cc_prim_env_shade_env;
            break;
    #endif
        case CC1(TEXEL0, SHADE, TEXEL0_ALPHA, SHADE):
            gdp_combine_cc = gdp_combine_cc_special1;
            gdp_triangle   = gdp_triangle_special;
            break;
    #ifdef APP_UNSM
        case CC1(PRIMITIVE, SHADE, TEXEL0, SHADE):
            gdp_combine_cc = gdp_combine_cc_special2;
            gdp_triangle   = gdp_triangle_special;
            gdp_texture_read_table[G_IM_FMT_IA << 2 | G_IM_SIZ_8b] =
                gdp_texture_read_ia8_special;
            break;
    #endif
    #ifdef APP_UNK4
        case CC1(SHADE, ENVIRONMENT, TEXEL0, ENVIRONMENT):
            gdp_combine_cc = gdp_combine_cc_special3;
            gdp_triangle   = gdp_triangle_special;
            break;
    #endif
        case CC1(PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT):
            gdp_combine_cc = gdp_combine_cc_special4;
            gdp_triangle   = gdp_triangle_special;
            break;
    #ifdef APP_UNK4
        case CC2(
            PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, COMBINED, 0, SHADE, 0
        ):
            gdp_combine_cc = gdp_combine_cc_special5;
            gdp_triangle   = gdp_triangle_special;
            break;
    #endif
    #ifdef APP_UNKT
        case CC1(1, ENVIRONMENT, TEXEL0, PRIMITIVE):
            gdp_combine_cc = gdp_combine_cc_special6;
            gdp_triangle   = gdp_triangle_special;
            break;
    #endif
        default:
            gdp_combine_cc = gdp_combine_cc_0;
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
    #ifndef APP_UNSM
        /* ??? */
        case AC1(PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT):
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
        case AC1(PRIMITIVE, SHADE, TEXEL0, SHADE):
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
}
