#define sleep_frame()   gspWaitForVBlank()

static float audio_mix[12] = {1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

static void app_init(void)
{
    osSetSpeedupEnable(true);
    video_update_size(400, 240);
    gfxInitDefault();
    consoleInit(GFX_BOTTOM, NULL);
    ndspInit();
    ndspSetOutputMode(NDSP_OUTPUT_STEREO);
    ndspChnSetInterp(0, NDSP_INTERP_LINEAR);
    ndspChnSetFormat(0, NDSP_FORMAT_STEREO_PCM16);
    ndspChnSetMix(0, audio_mix);
    ndspChnSetRate(0, AUDIO_FREQ);
    /* romfsInit(); */
}

static void app_exit(void)
{
    /* romfsExit(); */
    ndspExit();
    gfxExit();
}

static void app_update(void)
{
    if (!aptMainLoop()) exit(EXIT_SUCCESS);
}

static const u32 input_config[] =
{
    KEY_A,
    KEY_B | KEY_X,
    KEY_L,
    KEY_START,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    KEY_R,
    KEY_DUP    | KEY_CSTICK_UP,
    KEY_DDOWN  | KEY_CSTICK_DOWN,
    KEY_DLEFT  | KEY_CSTICK_LEFT,
    KEY_DRIGHT | KEY_CSTICK_RIGHT,
};

void input_update(void)
{
    circlePosition stick;
    u32 held;
    u32 down;
    uint i;
    hidScanInput();
    hidCircleRead(&stick);
    held = hidKeysHeld();
    down = hidKeysDown();
    if (held & KEY_Y)
    {
        if (down & KEY_START)   exit(EXIT_SUCCESS);
        if (down & KEY_SELECT)  sys_reset = true;
        if (down & KEY_ZL)      sys_save  = true;
        if (down & KEY_ZR)      sys_load  = true;
    }
    else
    {
        sys_fast = (down & KEY_SELECT) != 0;
    }
    os_cont_pad[0].button = 0;
    os_cont_pad[0].stick_x = stick.dx/2;
    os_cont_pad[0].stick_y = stick.dy/2;
    os_cont_pad[0].errno_ = 0;
    for (i = 0; i < 16; i++)
    {
        if (held & input_config[i]) os_cont_pad[0].button |= 0x8000 >> i;
    }
#ifdef APP_UNSM
    memcpy(&os_cont_pad[1], &os_cont_pad[0], sizeof(struct os_cont_pad));
#endif
}

static ndspWaveBuf audio_table[2] = {0};
static u8 audio_index = 0;

void audio_update(void *src, size_t size)
{
    ndspWaveBuf *wb = &audio_table[audio_index];
    if (wb->status == NDSP_WBUF_DONE || wb->status == NDSP_WBUF_FREE)
    {
        audio_index ^= 1;
        if (wb->data_pcm16 != NULL) linearFree(wb->data_pcm16);
        wb->data_pcm16 = linearAlloc(size);
        wb->nsamples = size / (2*sizeof(s16));
        wordswap(wb->data_pcm16, src, size);
        DSP_FlushDataCache(wb->data_pcm16, size);
        ndspChnWaveBufAdd(0, wb);
    }
}

s32 audio_size(void)
{
    return 0x200;
}
