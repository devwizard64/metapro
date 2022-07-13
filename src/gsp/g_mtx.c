#ifdef GSP_F3D
#define PUSH (flag & G_MTX_PUSH)
#endif
#ifdef GSP_F3DEX2
#define PUSH (!(flag & G_MTX_NOPUSH))
#endif
static void gsp_g_mtx(u32 w0, u32 w1)
{
    f32  mf[4][4];
    uint flag;
    gsp_flush_rect();
#ifdef APP_UNSM
#ifdef APP_J0
    if (w1 == 0x020144B0)
#endif
#ifdef APP_E0
    if (w1 == 0x02017310)
#endif
    {
    #if 1
        mtx_ortho(mf, video_l, video_r, 0, 240, 0, 2);
    #else
        mtx_ortho_bg(mf, 0, 320, 0, 240, 0, 2);
    #endif
    }
    else
#endif
    {
        mtx_read(&mf[0][0], gsp_addr(w1));
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
            memcpy(MP, mf, sizeof(mf));
        }
        else
        {
            f32 src[4][4];
            memcpy(src, MP, sizeof(src));
            mtx_cat(MP, mf, src);
        }
        gsp_flush_mp();
    }
    else
    {
        if (flag & G_MTX_LOAD)
        {
            if (PUSH) gsp_mtx_modelview++;
            memcpy(MM, mf, sizeof(mf));
        }
        else
        {
            f32 src[4][4];
            memcpy(src, MM, sizeof(src));
            if (PUSH) gsp_mtx_modelview++;
            mtx_cat(MM, mf, src);
        }
        gsp_flush_mm();
    #ifdef APP_UNK4
        mtx_write(gsp_mtx, &MM[0][0]);
    #endif
        gsp_new_light = true;
    }
#if defined(GSP_SWFOG) || defined(__NATIVE__)
    mtx_cat(gsp_mtx_mvp, MM, MP);
#endif
}
#undef PUSH
