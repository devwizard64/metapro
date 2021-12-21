#define VIDEO_SCALE     2

static u64 sleep_timer = 0;

static u64 sleep_time(void)
{
#ifdef WIN32
    LARGE_INTEGER count;
    LARGE_INTEGER freq;
    QueryPerformanceCounter(&count);
    QueryPerformanceFrequency(&freq);
    return 1000000000*count.QuadPart/freq.QuadPart;
#else
    struct timespec timespec;
    clock_gettime(CLOCK_MONOTONIC, &timespec);
    return 1000000000*timespec.tv_sec + timespec.tv_nsec;
#endif
}

static void sleep_frame(void)
{
    u64 time = sleep_time();
    if (time-sleep_timer > 50000000) sleep_timer = time;
    sleep_timer += 16666667;
    while (sleep_timer > time)
    {
        SDL_Delay(1);
        time = sleep_time();
    }
}

SDL_Window *window = NULL;
static SDL_Joystick *joystick = NULL;
static SDL_AudioDeviceID audio_device = 0;

static void app_init(void)
{
    SDL_AudioSpec spec;
    SDL_SetHint(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS, "1");
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_AUDIO);
    video_update_size(VIDEO_SCALE*320, VIDEO_SCALE*240);
    window = SDL_CreateWindow(
        "app", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        video_w, video_h, SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL
    );
    if (window == NULL)
    {
        eprint("could not create window (%s)\n", SDL_GetError());
    }
    if (SDL_GL_CreateContext(window) == NULL)
    {
        eprint("could not create context (%s)\n", SDL_GetError());
    }
    SDL_GL_SetSwapInterval(0);
    joystick = SDL_JoystickOpen(0);
    spec.freq     = AUDIO_FREQ;
    spec.format   = AUDIO_S16;
    spec.channels = 2;
    spec.samples  = 0;
    spec.callback = NULL;
    spec.userdata = NULL;
    audio_device = SDL_OpenAudioDevice(NULL, 0, &spec, NULL, 0);
    SDL_PauseAudioDevice(audio_device, 0);
}

static void app_exit(void)
{
    if (audio_device !=    0) SDL_CloseAudioDevice(audio_device);
    if (joystick     != NULL) SDL_JoystickClose(joystick);
    if (window       != NULL) SDL_DestroyWindow(window);
    SDL_Quit();
}

static void app_update(void)
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
            case SDL_QUIT:
                exit(EXIT_SUCCESS);
                break;
            case SDL_WINDOWEVENT:
                switch (event.window.event)
                {
                    case SDL_WINDOWEVENT_RESIZED:
                        SDL_ClearQueuedAudio(audio_device);
                        video_update_size(
                            event.window.data1, event.window.data2
                        );
                        break;
                }
                break;
            case SDL_KEYDOWN:
                switch (event.key.keysym.scancode)
                {
                    case SDL_SCANCODE_F1:   sys_reset = true;       break;
                    case SDL_SCANCODE_F4:   sys_fast ^= false^true; break;
                    case SDL_SCANCODE_F5:   sys_save = true;        break;
                    case SDL_SCANCODE_F7:   sys_load = true;        break;
                    default: break;
                }
                break;
        }
    }
    if (SDL_GetQueuedAudioSize(audio_device) > 16384)
    {
        SDL_ClearQueuedAudio(audio_device);
    }
}

static const s8 input_config[][2] =
{
    { 0,   0}, { 3,   0}, { 4,   0}, { 7,   0},
    { 8,   0}, { 9,   0}, {10,   0}, {11,   0},
    { 0,  78}, { 1, -78}, { 6,   0}, { 5,   0},
    { 4, -78}, { 4,  78}, { 3, -78}, { 3,  78},
};

void input_update(void)
{
    const u8 *keys = SDL_GetKeyboardState(NULL);
    uint mask;
    uint i;
    os_cont_pad[0].button  = 0;
    os_cont_pad[0].stick_x = 0;
    os_cont_pad[0].stick_y = 0;
    os_cont_pad[0].errno_  = 0;
    if (joystick != NULL)
    {
        for (mask = 0x8000, i = 0; i < 16; i++, mask >>= 1)
        {
            int id  = input_config[i][0];
            int mul = input_config[i][1];
            if (id != -1)
            {
                if (mul != 0)
                {
                    int axis = SDL_JoystickGetAxis(joystick, (u8)id);
                    axis = axis*mul/(0x100*100);
                    switch (mask)
                    {
                        case 0x0080: os_cont_pad[0].stick_x = axis; break;
                        case 0x0040: os_cont_pad[0].stick_y = axis; break;
                        default:
                            if (axis > 40) os_cont_pad[0].button |= mask;
                            break;
                    }
                }
                else
                {
                    if (SDL_JoystickGetButton(joystick, id))
                    {
                        os_cont_pad[0].button |= mask;
                    }
                }
            }
        }
    }
    if (keys[SDL_SCANCODE_X])       os_cont_pad[0].button |= 0x8000;
    if (keys[SDL_SCANCODE_C])       os_cont_pad[0].button |= 0x4000;
    if (keys[SDL_SCANCODE_Z])       os_cont_pad[0].button |= 0x2000;
    if (keys[SDL_SCANCODE_RETURN])  os_cont_pad[0].button |= 0x1000;
    if (keys[SDL_SCANCODE_I])       os_cont_pad[0].button |= 0x0800;
    if (keys[SDL_SCANCODE_K])       os_cont_pad[0].button |= 0x0400;
    if (keys[SDL_SCANCODE_J])       os_cont_pad[0].button |= 0x0200;
    if (keys[SDL_SCANCODE_L])       os_cont_pad[0].button |= 0x0100;
    if (keys[SDL_SCANCODE_Q])       os_cont_pad[0].button |= 0x0020;
    if (keys[SDL_SCANCODE_E])       os_cont_pad[0].button |= 0x0010;
    if (keys[SDL_SCANCODE_W])       os_cont_pad[0].button |= 0x0008;
    if (keys[SDL_SCANCODE_S])       os_cont_pad[0].button |= 0x0004;
    if (keys[SDL_SCANCODE_A])       os_cont_pad[0].button |= 0x0002;
    if (keys[SDL_SCANCODE_D])       os_cont_pad[0].button |= 0x0001;
    switch (keys[SDL_SCANCODE_LEFT] | keys[SDL_SCANCODE_RIGHT] << 1)
    {
        case 1: os_cont_pad[0].stick_x = -80;   break;
        case 2: os_cont_pad[0].stick_x =  80;   break;
    }
    switch (keys[SDL_SCANCODE_DOWN] | keys[SDL_SCANCODE_UP]    << 1)
    {
        case 1: os_cont_pad[0].stick_y = -80;   break;
        case 2: os_cont_pad[0].stick_y =  80;   break;
    }
#ifdef APP_UNSM
    memcpy(&os_cont_pad[1], &os_cont_pad[0], sizeof(OSContPad));
#endif
}

void audio_update(void *src, size_t size)
{
    void *data = malloc(size);
    wordswap(data, src, size);
    SDL_QueueAudio(audio_device, data, size);
    free(data);
}

s32 audio_size(void)
{
    return SDL_GetQueuedAudioSize(audio_device);
}
