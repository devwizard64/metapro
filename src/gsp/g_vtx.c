static void gsp_g_vtx(u32 w0, u32 w1)
{
#ifdef GSP_F3D
    uint count = w0 >> 20 & 0x0F;
    uint index = w0 >> 16 & 0x0F;
    uint size  = w0 >>  0 & 0xFFFF;
#endif
#ifdef GSP_F3DEX2
    uint count = w0 >> 12 & 0xFF;
    uint size  = w0 >>  8 & 0xFFFF;
    uint end   = w0 >>  1 & 0x7F;
    uint index = end-count;
#endif
    struct vtx_t  *vtx  = &gsp_vtx_buf[index];
    struct vtxf_t *vtxf = &gsp_vtxf_buf[index];
    memcpy(vtx, gsp_addr(w1), size);
    if (gsp_light_new)
    {
        uint i;
        gsp_light_new = false;
        for (i = gsp_lookat ? 0 : 2; i < gsp_light_no+1; i++)
        {
            struct light_t  *light;
            struct lightf_t *lightf;
            f32 x;
            f32 y;
            f32 z;
            f32 nx;
            f32 ny;
            f32 nz;
            f32 d;
            light  = &gsp_light_buf[i];
            lightf = &gsp_lightf_buf[i];
            lightf->r = light->col[0].r;
            lightf->g = light->col[0].g;
            lightf->b = light->col[0].b;
            x = light->x;
            y = light->y;
            z = light->z;
            nx = IDOT3(MM, 0);
            ny = IDOT3(MM, 1);
            nz = IDOT3(MM, 2);
            d = sqrtf(nx*nx + ny*ny + nz*nz);
            if (d > 0)
            {
                d = (1.0F/0x80) / d;
                nx *= d;
                ny *= d;
                nz *= d;
            }
            lightf->x = nx;
            lightf->y = ny;
            lightf->z = nz;
        }
    }
    do
    {
        if (gsp_geometry_mode & G_TEXTURE_GEN)
        {
            f32 x = vtx->r;
            f32 y = vtx->g;
            f32 z = vtx->b;
            if (gsp_lookat)
            {
                vtxf->u =
                    gsp_lightf_buf[0].x*x +
                    gsp_lightf_buf[0].y*y +
                    gsp_lightf_buf[0].z*z;
                vtxf->v =
                    gsp_lightf_buf[1].x*x +
                    gsp_lightf_buf[1].y*y +
                    gsp_lightf_buf[1].z*z;
                vtxf->u = 0x4000 * (1+vtxf->u);
                vtxf->v = 0x4000 * (1+vtxf->v);
            }
            else
            {
                f32 nx = MDOT3(MM, 0);
                f32 ny = MDOT3(MM, 1);
                f32 nz = MDOT3(MM, 2);
                f32 d = sqrtf(nx*nx + ny*ny + nz*nz);
                if (d > 0)
                {
                    d = 0x4000 / d;
                    nx *= d;
                    ny *= d;
                }
                vtxf->u = 0x4000 + nx;
                vtxf->v = 0x4000 + ny;
            }
        }
        else
        {
            vtxf->u = vtx->u;
            vtxf->v = vtx->v;
        }
        vtxf->u *= gsp_texture_vscale[0];
        vtxf->v *= gsp_texture_vscale[1];
        if (gsp_texture_filter != GL_NEAREST)
        {
            vtxf->u += 32*0.5F;
            vtxf->v += 32*0.5F;
        }
        if (gsp_geometry_mode & G_LIGHTING)
        {
            f32  x;
            f32  y;
            f32  z;
            uint n;
            uint r;
            uint g;
            uint b;
            uint i;
            x = vtx->r;
            y = vtx->g;
            z = vtx->b;
            n = gsp_light_no+1;
            r = gsp_light_buf[n].col[0].r;
            g = gsp_light_buf[n].col[0].g;
            b = gsp_light_buf[n].col[0].b;
            for (i = 2; i < n; i++)
            {
                struct lightf_t *lightf = &gsp_lightf_buf[i];
                f32 d = lightf->x*x + lightf->y*y + lightf->z*z;
                if (d > 0)
                {
                    r += d*lightf->r;
                    g += d*lightf->g;
                    b += d*lightf->b;
                }
            }
            if (r > 0xFF)
            {
                r = 0xFF;
            }
            if (g > 0xFF)
            {
                g = 0xFF;
            }
            if (b > 0xFF)
            {
                b = 0xFF;
            }
            vtxf->shade[0] = r;
            vtxf->shade[1] = g;
            vtxf->shade[2] = b;
        }
        else
        {
            vtxf->shade[0] = vtx->r;
            vtxf->shade[1] = vtx->g;
            vtxf->shade[2] = vtx->b;
        }
    #ifdef GSP_FOG
        if (gsp_geometry_mode & G_FOG)
        {
            f32 x;
            f32 y;
            f32 z;
            f32 nz;
            f32 nw;
            int a;
            x = vtx->x;
            y = vtx->y;
            z = vtx->z;
            nz = MDOT4(gsp_mtxf_mvp, 2);
            nw = MDOT4(gsp_mtxf_mvp, 3);
            z = nz/nw;
        #ifdef GEKKO
            z = 1 + 2*z;
        #endif
            if (z > 1)
            {
                z = -1;
            }
            a = gsp_fog_o + (s32)(gsp_fog_m*z);
            if (a < 0x00)
            {
                a = 0x00;
            }
            if (a > 0xFF)
            {
                a = 0xFF;
            }
            vtxf->shade[3] = a;
        }
        else
    #endif
        {
            vtxf->shade[3] = vtx->a;
        }
        vtx++;
        vtxf++;
    }
#ifdef GSP_F3D
    while (count-- > 0);
#endif
#ifdef GSP_F3DEX2
    while (--count > 0);
#endif
}

#ifdef _3DS
#ifdef _DEBUG
static void gsp_g_vtx_stub(unused u32 w0, unused u32 w1)
{
}
#else
#define gsp_g_vtx_stub  NULL
#endif
#endif
