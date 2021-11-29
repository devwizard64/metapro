static SDL_AudioDeviceID audio_device = 0;

static s32 audio_size(void)
{
    return SDL_GetQueuedAudioSize(audio_device);
}

static void audio_init(void)
{
    SDL_AudioSpec spec;
    SDL_Init(SDL_INIT_AUDIO);
    spec.freq     = 32000;
    spec.format   = AUDIO_S16;
    spec.channels = 2;
    spec.samples  = 0;
    spec.callback = NULL;
    spec.userdata = NULL;
    audio_device = SDL_OpenAudioDevice(NULL, 0, &spec, NULL, 0);
    SDL_PauseAudioDevice(audio_device, 0);
}

static void audio_exit(void)
{
    if (audio_device != 0) SDL_CloseAudioDevice(audio_device);
}

static void audio_update(void *src, size_t size)
{
    void *data = malloc(size);
    __WORDSWAP(data, src, size);
    SDL_QueueAudio(audio_device, data, size);
    free(data);
}
