static void video_init(void)
{
    video_update_size(400, 240);
    gfxInitDefault();
    consoleInit(GFX_BOTTOM, NULL);
}

static void video_exit(void)
{
    gfxExit();
}
