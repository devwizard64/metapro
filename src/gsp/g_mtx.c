#ifdef GSP_F3D
#define PUSH (flag & G_MTX_PUSH)
#endif
#ifdef GSP_F3DEX2
#define PUSH (!(flag & G_MTX_NOPUSH))
#endif
static void gsp_g_mtx(u32 w0, u32 w1)
{
    f32  mtxf[4][4];
    uint flag;
    gsp_flush_rect();
#ifdef APP_UNSM
    if (w1 == 0x02017310)
    {
    #if 1
        mtxf_ortho(
            mtxf, lib_viewport_l, lib_viewport_r, 0, 240, 0, 2
        );
    #else
        f32 h = 120 * 4/3 * lib_video_h/lib_video_w;
        mtxf_ortho(mtxf, 0, 320, 120-h, 120+h, 0, 2);
    #endif
    }
    else
#endif
    {
        mtx_read(&mtxf[0][0], gsp_addr(w1));
    }
#ifdef GSP_F3D
    flag = w0 >> 16;
#endif
#ifdef GSP_F3DEX2
    flag = w0 >>  0;
#endif
    if (flag & G_MTX_PROJECTION)
    {
        if (flag & G_MTX_LOAD)
        {
            memcpy(MP, mtxf, sizeof(mtxf));
        }
        else
        {
            f32 src[4][4];
            memcpy(src, MP, sizeof(src));
            mtxf_mul(MP, mtxf, src);
        }
        gsp_change |= CHANGE_MTXF_PROJECTION;
    }
    else
    {
        if (flag & G_MTX_LOAD)
        {
            if (PUSH)
            {
                gsp_mtxf_modelview++;
            }
            memcpy(MM, mtxf, sizeof(mtxf));
        }
        else
        {
            f32 src[4][4];
            memcpy(src, MM, sizeof(src));
            if (PUSH)
            {
                gsp_mtxf_modelview++;
            }
            mtxf_mul(MM, mtxf, src);
        }
    #ifdef GSP_F3DEX2
        mtx_write(gsp_mtx, &MM[0][0]);
    #endif
        gsp_change |= CHANGE_MTXF_MODELVIEW;
        gsp_light_new = true;
    }
#ifdef GSP_FOG
    mtxf_mul(gsp_mtxf_mvp, MM, MP);
#endif
}
#undef PUSH
