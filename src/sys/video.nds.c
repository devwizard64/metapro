static void video_init(void)
{
    video_update_size(256, 192);
    consoleDemoInit();
    videoSetMode(MODE_0_3D);
}

static void video_exit(void)
{
}
