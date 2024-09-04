#define sleep_frame()   swiWaitForVBlank()

static void app_init(void)
{
	video_resize_1(256, 192);
	consoleDemoInit();
	videoSetMode(MODE_0_3D);
	soundEnable();
	fatInitDefault();
}

static void app_exit(void)
{
}

static void app_update(void)
{
}

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

void input_update(void)
{
	int i;
	uint held;
	uint down;
	scanKeys();
	held = keysHeld();
	down = keysDown();
	if (held & KEY_Y)
	{
		if (down & KEY_START)   exit(0);
		if (down & KEY_SELECT)  sys_reset = TRUE;
	}
	else
	{
		if (down & KEY_SELECT)  sys_fast ^= FALSE^TRUE;
	}
	os_cont_pad[0].button  = 0;
	os_cont_pad[0].stick_x = 0;
	os_cont_pad[0].stick_y = 0;
	os_cont_pad[0].errno_  = 0;
	if (held & KEY_LEFT)    os_cont_pad[0].stick_x = -80;
	if (held & KEY_RIGHT)   os_cont_pad[0].stick_x =  80;
	if (held & KEY_DOWN)    os_cont_pad[0].stick_y = -80;
	if (held & KEY_UP)      os_cont_pad[0].stick_y =  80;
	for (i = 0; i < 16; i++)
	{
		if (held & input_config[i]) os_cont_pad[0].button |= 0x8000 >> i;
	}
#ifdef APP_UNSM
	memcpy(&os_cont_pad[1], &os_cont_pad[0], sizeof(OSContPad));
#endif
}

struct audio
{
	s16 *data;
	int  id;
};

static struct audio audio_table[2][2] = {0};
static u8 audio_index = 0;

void audio_update(void *src, size_t size)
{
	uint i;
	s16 *data = src;
	struct audio *audio = audio_table[audio_index];
	audio_index ^= 1;
	if (audio[0].data)
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
		audio[0].data, SoundFormat_16Bit, size, AUDIO_FREQ, 127,   0, TRUE, 0
	);
	audio[1].id = soundPlaySample(
		audio[1].data, SoundFormat_16Bit, size, AUDIO_FREQ, 127, 127, TRUE, 0
	);
}

s32 audio_size(void)
{
	return 512;
}
