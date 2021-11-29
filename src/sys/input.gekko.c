static const u16 input_config[] =
{
    PAD_BUTTON_A,
    PAD_BUTTON_B | PAD_BUTTON_Y,
    PAD_TRIGGER_L,
    PAD_BUTTON_START,
    PAD_BUTTON_UP,
    PAD_BUTTON_DOWN,
    PAD_BUTTON_LEFT,
    PAD_BUTTON_RIGHT,
    0,
    0,
    PAD_TRIGGER_Z,
    PAD_TRIGGER_R,
    0,
    0,
    0,
    0,
};

#ifdef __WII__
static void input_power(void)
{
    SYS_ResetSystem(SYS_POWEROFF, 0, 0);
}
#endif

static void input_init(void)
{
    PAD_Init();
#ifdef __WII__
    SYS_SetPowerCallback(input_power);
#endif
}

static void input_exit(void)
{
}

static void input_update(void)
{
    uint mask = PAD_ScanPads();
    uint pad;
    uint i;
    if (SYS_ResetButtonDown()) sys_reset = true;
    if (PAD_ButtonsHeld(0) & PAD_TRIGGER_Z)
    {
        uint down = PAD_ButtonsDown(0);
        if (down & PAD_BUTTON_START)    exit(EXIT_SUCCESS);
        if (down & PAD_BUTTON_LEFT)     sys_save = true;
        if (down & PAD_BUTTON_RIGHT)    sys_load = true;
        sys_fast = (down & PAD_BUTTON_X) != 0;
    }
    for (pad = 0; pad < 4; pad++)
    {
        int  stick_x    = PAD_StickX(pad);
        int  stick_y    = PAD_StickY(pad);
        int  substick_x = PAD_SubStickX(pad);
        int  substick_y = PAD_SubStickY(pad);
        uint held       = PAD_ButtonsHeld(pad);
        os_pad[pad].button  = 0;
        os_pad[pad].stick_x = stick_x * 78/100;
        os_pad[pad].stick_y = stick_y * 78/100;
        os_pad[pad].errno_  = (mask & (1 << pad)) ? 0 : CONT_NO_RESPONSE_ERROR;
        if (substick_y >  50) os_pad[pad].button |= 0x0008;
        if (substick_y < -50) os_pad[pad].button |= 0x0004;
        if (substick_x < -50) os_pad[pad].button |= 0x0002;
        if (substick_x >  50) os_pad[pad].button |= 0x0001;
        for (i = 0; i < 16; i++)
        {
            if (held & input_config[i]) os_pad[pad].button |= 0x8000 >> i;
        }
    }
}
