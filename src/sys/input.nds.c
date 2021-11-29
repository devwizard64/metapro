static const u16 input_config[] =
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
    0,
    0,
    0,
    0,
};

static void input_init(void)
{
}

static void input_exit(void)
{
}

static void input_update(void)
{
    uint held;
    uint down;
    uint i;
    scanKeys();
    held = keysHeld();
    down = keysDown();
    if (held & KEY_Y)
    {
        if (down & KEY_START)   exit(EXIT_SUCCESS);
        if (down & KEY_SELECT)  sys_reset = true;
    }
    else
    {
        sys_fast = (down & KEY_SELECT) != 0;
    }
    os_pad[0].button  = 0;
    os_pad[0].stick_x = 0;
    os_pad[0].stick_y = 0;
    os_pad[0].errno_  = 0;
    if (held & KEY_LEFT)    os_pad[0].stick_x = -80;
    if (held & KEY_RIGHT)   os_pad[0].stick_x =  80;
    if (held & KEY_DOWN)    os_pad[0].stick_y = -80;
    if (held & KEY_UP)      os_pad[0].stick_y =  80;
    for (i = 0; i < 16; i++)
    {
        if (held & input_config[i]) os_pad[0].button |= 0x8000 >> i;
    }
#ifdef APP_UNSM
    memcpy(&os_pad[1], &os_pad[0], sizeof(struct os_pad));
#endif
}
