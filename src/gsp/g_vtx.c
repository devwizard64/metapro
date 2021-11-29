static void gsp_g_vtx(u32 w0, u32 w1)
{
#ifdef GSP_F3D
#ifdef GSP_F3DEX
    uint index = w0 >> 17 & 0x7F;
    uint count = w0 >> 10 & 0x3F;
    uint size  = w0 >>  0 & 0x03FF;
    size++;
#else
    uint count = w0 >> 20 & 0x0F;
    uint index = w0 >> 16 & 0x0F;
    uint size  = w0 >>  0 & 0xFFFF;
#endif
#endif
#ifdef GSP_F3DEX2
    uint count = w0 >> 12 & 0xFF;
    uint size  = w0 >>  8 & 0xFFFF;
    uint end   = w0 >>  1 & 0x7F;
    uint index = end-count;
#endif
    struct vtx  *v  = &gsp_vtx_buf[index];
    struct vtxf *vf = &gsp_vtxf_buf[index];
    memcpy(v, gsp_addr(w1), size);
    if (gsp_new_light)
    {
        uint i;
        gsp_new_light = false;
        for (i = 2; i < gsp_light_no; i++)
        {
            struct light  *l  = &gsp_light_buf[i];
            struct lightf *lf = &gsp_lightf_buf[i];
            float x = l->x;
            float y = l->y;
            float z = l->z;
            float nx = IDOT3(MM, 0);
            float ny = IDOT3(MM, 1);
            float nz = IDOT3(MM, 2);
            float d = sqrtf(nx*nx + ny*ny + nz*nz);
            if (d > 0)
            {
                d = (1.0F/0x80) / d;
                nx *= d;
                ny *= d;
                nz *= d;
            }
            lf->x = nx;
            lf->y = ny;
            lf->z = nz;
            lf->r = l->col[0].r;
            lf->g = l->col[0].g;
            lf->b = l->col[0].b;
        }
    }
    do
    {
        int s;
        int t;
        if (gsp_geometry_mode & G_TEXTURE_GEN)
        {
            if (gsp_lookat)
            {
            #define LOOKATY gsp_light_buf[0]
            #define LOOKATX gsp_light_buf[1]
                s = LOOKATX.x*v->r + LOOKATX.y*v->g + LOOKATX.z*v->b;
                t = LOOKATY.x*v->r + LOOKATY.y*v->g + LOOKATY.z*v->b;
            #undef LOOKATY
            #undef LOOKATX
            }
            else
            {
                float x = v->r;
                float y = v->g;
                float z = v->b;
                float nx = MDOT3(MM, 0);
                float ny = MDOT3(MM, 1);
                float nz = MDOT3(MM, 2);
                float d = sqrtf(nx*nx + ny*ny + nz*nz);
                if (d > 0)
                {
                    d = 0x4000 / d;
                    nx *= d;
                    ny *= d;
                }
                s = ny;
                t = nx;
            }
            s += 0x4000;
            t += 0x4000;
        }
        else
        {
            s = v->s;
            t = v->t;
        }
        s = (gsp_texture_scale[0]*s + 0x8000) >> 16;
        t = (gsp_texture_scale[1]*t + 0x8000) >> 16;
        if (gdp_tf != GDP_TF_POINT)
        {
            s += 32/2;
            t += 32/2;
        }
        vf->s = s;
        vf->t = t;
        if (gsp_geometry_mode & G_LIGHTING)
        {
            float x = v->r;
            float y = v->g;
            float z = v->b;
            uint  r = gsp_light_buf[gsp_light_no].col[0].r;
            uint  g = gsp_light_buf[gsp_light_no].col[0].g;
            uint  b = gsp_light_buf[gsp_light_no].col[0].b;
            uint  i;
            for (i = 2; i < gsp_light_no; i++)
            {
                struct lightf *lf = &gsp_lightf_buf[i];
                float d = lf->x*x + lf->y*y + lf->z*z;
                if (d > 0)
                {
                    r += d*lf->r;
                    g += d*lf->g;
                    b += d*lf->b;
                }
            }
            if (r > 0xFF) r = 0xFF;
            if (g > 0xFF) g = 0xFF;
            if (b > 0xFF) b = 0xFF;
            vf->shade[0] = r;
            vf->shade[1] = g;
            vf->shade[2] = b;
        }
        else
        {
            vf->shade[0] = v->r;
            vf->shade[1] = v->g;
            vf->shade[2] = v->b;
        }
    #if defined(GSP_SWFOG) || defined(__NATIVE__)
        if (gsp_geometry_mode & G_FOG)
        {
            float x = v->x;
            float y = v->y;
            float z = v->z;
            float nz = MDOT4(gsp_mtxf_mvp, 2);
            float nw = MDOT4(gsp_mtxf_mvp, 3);
            int   a;
            z = nz/nw;
            if (z > 1) z = -1;
            a = gsp_fog_o + (int)(gsp_fog_m*z);
            if (a < 0x00) a = 0x00;
            if (a > 0xFF) a = 0xFF;
            vf->shade[3] = a;
        }
        else
    #endif
        {
            vf->shade[3] = v->a;
        }
        v++;
        vf++;
    }
#ifdef GSP_F3DEX
    while (--count > 0);
#else
    while (count-- > 0);
#endif
}
