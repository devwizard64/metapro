static float audio_mix[12] = {1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static ndspWaveBuf audio_table[2] = {0};
static u8 audio_index = 0;

static s32 audio_size(void)
{
    return 0x200;
}

static void audio_init(void)
{
    ndspInit();
    ndspSetOutputMode(NDSP_OUTPUT_STEREO);
    ndspChnSetInterp(0, NDSP_INTERP_LINEAR);
    ndspChnSetFormat(0, NDSP_FORMAT_STEREO_PCM16);
    ndspChnSetMix(0, audio_mix);
    ndspChnSetRate(0, 32000);
}

static void audio_exit(void)
{
    ndspExit();
}

static void audio_update(void *src, size_t size)
{
    ndspWaveBuf *wb = &audio_table[audio_index];
    if (wb->status == NDSP_WBUF_DONE || wb->status == NDSP_WBUF_FREE)
    {
        audio_index ^= 1;
        if (wb->data_pcm16 != NULL) linearFree(wb->data_pcm16);
        wb->data_pcm16 = linearAlloc(size);
        wb->nsamples = size / (2*sizeof(s16));
        __WORDSWAP(wb->data_pcm16, src, size);
        DSP_FlushDataCache(wb->data_pcm16, size);
        ndspChnWaveBufAdd(0, wb);
    }
}
