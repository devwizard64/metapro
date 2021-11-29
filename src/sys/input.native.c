static const s8 input_config[][2] =
{
    { 0,   0}, { 3,   0}, { 4,   0}, { 7,   0},
    { 8,   0}, { 9,   0}, {10,   0}, {11,   0},
    { 0,  78}, { 1, -78}, { 6,   0}, { 5,   0},
    { 4, -78}, { 4,  78}, { 3, -78}, { 3,  78},
};

static SDL_Joystick *joystick = NULL;

static void input_init(void)
{
    SDL_SetHint(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS, "1");
    SDL_Init(SDL_INIT_JOYSTICK);
    joystick = SDL_JoystickOpen(0);
}

static void input_exit(void)
{
    if (joystick != NULL) SDL_JoystickClose(joystick);
}

static void input_update(void)
{
    const u8 *keys = SDL_GetKeyboardState(NULL);
    uint mask;
    uint i;
    os_pad[0].button  = 0;
    os_pad[0].stick_x = 0;
    os_pad[0].stick_y = 0;
    os_pad[0].errno_  = 0;
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
                        case 0x0080: os_pad[0].stick_x = axis; break;
                        case 0x0040: os_pad[0].stick_y = axis; break;
                        default:
                            if (axis > 40) os_pad[0].button |= mask;
                            break;
                    }
                }
                else
                {
                    if (SDL_JoystickGetButton(joystick, id))
                    {
                        os_pad[0].button |= mask;
                    }
                }
            }
        }
    }
    if (keys[SDL_SCANCODE_X])       os_pad[0].button |= 0x8000;
    if (keys[SDL_SCANCODE_C])       os_pad[0].button |= 0x4000;
    if (keys[SDL_SCANCODE_Z])       os_pad[0].button |= 0x2000;
    if (keys[SDL_SCANCODE_RETURN])  os_pad[0].button |= 0x1000;
    if (keys[SDL_SCANCODE_I])       os_pad[0].button |= 0x0800;
    if (keys[SDL_SCANCODE_K])       os_pad[0].button |= 0x0400;
    if (keys[SDL_SCANCODE_J])       os_pad[0].button |= 0x0200;
    if (keys[SDL_SCANCODE_L])       os_pad[0].button |= 0x0100;
    if (keys[SDL_SCANCODE_Q])       os_pad[0].button |= 0x0020;
    if (keys[SDL_SCANCODE_E])       os_pad[0].button |= 0x0010;
    if (keys[SDL_SCANCODE_W])       os_pad[0].button |= 0x0008;
    if (keys[SDL_SCANCODE_S])       os_pad[0].button |= 0x0004;
    if (keys[SDL_SCANCODE_A])       os_pad[0].button |= 0x0002;
    if (keys[SDL_SCANCODE_D])       os_pad[0].button |= 0x0001;
    switch (keys[SDL_SCANCODE_LEFT] | keys[SDL_SCANCODE_RIGHT] << 1)
    {
        case 1: os_pad[0].stick_x = -80;   break;
        case 2: os_pad[0].stick_x =  80;   break;
    }
    switch (keys[SDL_SCANCODE_DOWN] | keys[SDL_SCANCODE_UP]    << 1)
    {
        case 1: os_pad[0].stick_y = -80;   break;
        case 2: os_pad[0].stick_y =  80;   break;
    }
#ifdef APP_UNSM
    memcpy(&os_pad[1], &os_pad[0], sizeof(struct os_pad));
#endif
}
