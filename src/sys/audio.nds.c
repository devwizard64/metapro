struct audio
{
    s16 *data;
    int  id;
};

static struct audio audio_table[2][2] = {0};
static u8 audio_index = 0;

static s32 audio_size(void)
{
    return 0x200;
}

static void audio_init(void)
{
    soundEnable();
}

static void audio_exit(void)
{
}

static void audio_update(void *src, size_t size)
{
    s16 *data = src;
    struct audio *audio;
    uint i;
    audio = audio_table[audio_index];
    audio_index ^= 1;
    if (audio[0].data != NULL)
    {
        soundKill(audio[0].id);
        soundKill(audio[1].id);
        free(audio[0].data);
        free(audio[1].data);
    }
    size /= 2;
    audio[0].data = malloc(size);
    audio[1].data = malloc(size);
    for (i = 0; i < size; i += 2)
    {
        audio[0].data[i/2] = data[i+1];
        audio[1].data[i/2] = data[i+0];
    }
    audio[0].id = soundPlaySample(
        audio[0].data, SoundFormat_16Bit, size, 32000, 127,   0, true, 0
    );
    audio[1].id = soundPlaySample(
        audio[1].data, SoundFormat_16Bit, size, 32000, 127, 127, true, 0
    );
}
