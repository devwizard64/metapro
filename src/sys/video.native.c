#define VIDEO_SCALE     2

SDL_Window *window = NULL;

static void video_init(void)
{
    SDL_Init(SDL_INIT_VIDEO);
    video_update_size(VIDEO_SCALE*320 /*400*/, VIDEO_SCALE*240);
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
}

static void video_exit(void)
{
    if (window != NULL) SDL_DestroyWindow(window);
}
