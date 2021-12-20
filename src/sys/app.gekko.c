#define sleep_frame()   VIDEO_WaitVSync()

void *framebuffer = NULL;
static GXRModeObj *video_rmode = NULL;

#ifdef __WII__
static void input_power(void)
{
    SYS_ResetSystem(SYS_POWEROFF, 0, 0);
}
#endif

static void app_init(void)
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
    PAD_Init();
#ifdef __WII__
    SYS_SetPowerCallback(input_power);
#endif
    AUDIO_Init(NULL);
    AUDIO_SetDSPSampleRate(AI_SAMPLERATE_32KHZ);
    fatInitDefault();
}

static void app_exit(void)
{
    AUDIO_StopDMA();
}

static void app_update(void)
{
}

static const u16 input_config[] =
{
    PAD_BUTTON_A,
    PAD_BUTTON_B | PAD_BUTTON_Y,
    PAD_TRIGGER_L,
    PAD_BUTTON_START,
    PAD_BUTTON_UP,
    PAD_BUTTON_DOWN,
    PAD_BUTTON_LEFT,
    PAD_BUTTON_RIGHT,
    0,
    0,
    PAD_TRIGGER_Z,
    PAD_TRIGGER_R,
    0,
    0,
    0,
    0,
};

void input_update(void)
{
    uint mask = PAD_ScanPads();
    uint pad;
    uint i;
    if (SYS_ResetButtonDown()) sys_reset = true;
    if (PAD_ButtonsHeld(0) & PAD_TRIGGER_Z)
    {
        uint down = PAD_ButtonsDown(0);
        if (down & PAD_BUTTON_START)    exit(EXIT_SUCCESS);
        if (down & PAD_BUTTON_LEFT)     sys_save = true;
        if (down & PAD_BUTTON_RIGHT)    sys_load = true;
        sys_fast = (down & PAD_BUTTON_X) != 0;
    }
    for (pad = 0; pad < 4; pad++)
    {
        int  stick_x    = PAD_StickX(pad);
        int  stick_y    = PAD_StickY(pad);
        int  substick_x = PAD_SubStickX(pad);
        int  substick_y = PAD_SubStickY(pad);
        uint held       = PAD_ButtonsHeld(pad);
        os_cont_pad[pad].button  = 0;
        os_cont_pad[pad].stick_x = stick_x * 78/100;
        os_cont_pad[pad].stick_y = stick_y * 78/100;
        os_cont_pad[pad].errno_  =
            (mask & (1 << pad)) ? 0 : CONT_NO_RESPONSE_ERROR;
        if (substick_y >  50) os_cont_pad[pad].button |= 0x0008;
        if (substick_y < -50) os_cont_pad[pad].button |= 0x0004;
        if (substick_x < -50) os_cont_pad[pad].button |= 0x0002;
        if (substick_x >  50) os_cont_pad[pad].button |= 0x0001;
        for (i = 0; i < 16; i++)
        {
            if (held & input_config[i]) os_cont_pad[pad].button |= 0x8000 >> i;
        }
    }
}

static u8 *audio_table[2] = {0};
static u8  audio_index = 0;

void audio_update(void *src, size_t size)
{
    u8 **data;
    size_t len;
    data = &audio_table[audio_index];
    audio_index ^= 1;
    len = (size+0x1F) & ~0x1F;
    if (*data != NULL) free(*data);
    *data = memalign(0x20, len);
    wordswap(*data, src, size);
    memset(*data+size, 0x00, len-size);
    DCFlushRange(*data, len);
    AUDIO_InitDMA((u32)*data, len);
    AUDIO_StartDMA();
}

s32 audio_size(void)
{
    return AUDIO_GetDMABytesLeft();
}
