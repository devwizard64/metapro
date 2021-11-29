void *framebuffer = NULL;
static GXRModeObj *video_rmode = NULL;

static void video_init(void)
{
    VIDEO_Init();
    video_rmode = VIDEO_GetPreferredMode(NULL);
    video_update_wh(video_rmode->fbWidth, video_rmode->efbHeight);
#ifdef __WII__
    if (CONF_GetAspectRatio() == CONF_ASPECT_16_9)
    {
        video_rmode->viXOrigin = (VI_MAX_WIDTH_NTSC-678)/2;
        video_rmode->viWidth = 678;
        video_aspect = (float)16/9;
    }
    else
#endif
    {
        video_aspect = (float)4/3;
    }
    video_update_lr();
    VIDEO_Configure(video_rmode);
    framebuffer = MEM_K0_TO_K1(SYS_AllocateFramebuffer(video_rmode));
    VIDEO_SetNextFramebuffer(framebuffer);
    VIDEO_SetBlack(false);
    VIDEO_Flush();
    CON_Init(
        framebuffer, 20, 20, video_rmode->fbWidth, video_rmode->xfbHeight,
        2*video_rmode->fbWidth
    );
    GX_Init(MEM_K0_TO_K1(memalign(0x20, 0x40000)), 0x40000);
    GX_SetCopyClear((GXColor){0x00, 0x00, 0x00, 0xFF}, GX_MAX_Z24);
    GX_SetCopyFilter(
        video_rmode->aa, video_rmode->sample_pattern, GX_TRUE,
        video_rmode->vfilter
    );
    GX_SetFieldMode(
        video_rmode->field_rendering,
        video_rmode->viHeight == 2*video_rmode->xfbHeight
    );
    GX_SetDispCopyYScale((float)video_rmode->xfbHeight/video_rmode->efbHeight);
    GX_SetDispCopyDst(video_rmode->fbWidth, video_rmode->xfbHeight);
    GX_SetDispCopySrc(0, 0, video_rmode->fbWidth, video_rmode->efbHeight);
    GX_SetDispCopyGamma(GX_GM_1_0);
}

static void video_exit(void)
{
}
