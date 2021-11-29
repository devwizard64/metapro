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

static void input_init(void)
{
}

static void input_exit(void)
{
}

static void input_update(void)
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
    os_pad[0].button = 0;
    os_pad[0].stick_x = stick.dx/2;
    os_pad[0].stick_y = stick.dy/2;
    os_pad[0].errno_ = 0;
    for (i = 0; i < 16; i++)
    {
        if (held & input_config[i]) os_pad[0].button |= 0x8000 >> i;
    }
#ifdef APP_UNSM
    memcpy(&os_pad[1], &os_pad[0], sizeof(struct os_pad));
#endif
}
