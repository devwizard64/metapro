static u8 *audio_table[2] = {0};
static u8  audio_index = 0;

static s32 audio_size(void)
{
    return AUDIO_GetDMABytesLeft();
}

static void audio_init(void)
{
    AUDIO_Init(NULL);
    AUDIO_SetDSPSampleRate(AI_SAMPLERATE_32KHZ);
}

static void audio_exit(void)
{
    AUDIO_StopDMA();
}

static void audio_update(void *src, size_t size)
{
    u8 **data;
    size_t len;
    data = &audio_table[audio_index];
    audio_index ^= 1;
    len = (size+0x1F) & ~0x1F;
    if (*data != NULL) free(*data);
    *data = memalign(0x20, len);
    __WORDSWAP(*data, src, size);
    memset(*data+size, 0x00, len-size);
    DCFlushRange(*data, len);
    AUDIO_InitDMA((u32)*data, len);
    AUDIO_StartDMA();
}
