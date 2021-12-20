static u8 *contdemo_data = NULL;
static u8 *contdemo      = NULL;
static size_t contdemo_size = 0;

static void contdemo_init(void)
{
#ifdef INPUT_WRITE
    contdemo_data = contdemo = malloc(4*30*60*60*2);
#else
    FILE *f = fopen(PATH_INPUT, "rb");
    if (f != NULL)
    {
        fseek(f, 0, SEEK_END);
        contdemo_size = ftell(f);
        fseek(f, 0, SEEK_SET);
        contdemo_data = contdemo = malloc(contdemo_size);
        fread(contdemo_data, 1, contdemo_size, f);
        fclose(f);
    }
#endif
}

static void contdemo_exit(void)
{
    if (contdemo_data != NULL) free(contdemo_data);
}

static void contdemo_save(void)
{
#ifdef INPUT_WRITE
    FILE *f = fopen(PATH_INPUT, "wb");
    if (f != NULL)
    {
        fwrite(contdemo_data, 1, contdemo-contdemo_data, f);
        fclose(f);
    }
    else
    {
        wdebug("could not write '" PATH_INPUT "'\n");
    }
#endif
}

static void contdemo_load(void)
{
#ifdef INPUT_WRITE
    FILE *f = fopen(PATH_INPUT, "rb");
    if (f != NULL)
    {
        size_t size;
        fseek(f, 0, SEEK_END);
        size = ftell(f);
        fseek(f, 0, SEEK_SET);
        fread(contdemo_data, 1, size, f);
        fclose(f);
        contdemo = (u8 *)contdemo_data + size;
    }
    else
    {
        wdebug("could not read '" PATH_INPUT "'\n");
    }
#endif
}

void contdemo_update(void)
{
#ifdef INPUT_WRITE
    contdemo[0] = os_cont_pad[0].button >> 8;
    contdemo[1] = os_cont_pad[0].button >> 0;
    contdemo[2] = os_cont_pad[0].stick_x;
    contdemo[3] = os_cont_pad[0].stick_y;
    contdemo += 4;
#else
    if (contdemo_size > 0)
    {
        os_cont_pad[0].button  = contdemo[0] << 8 | contdemo[1];
        os_cont_pad[0].stick_x = contdemo[2];
        os_cont_pad[0].stick_y = contdemo[3];
        pdebug(
            "X:%+4d  Y:%+4d  [%c%c%c|%c%c%c%c%c%c|%c%c%c]  F:%d\n",
            os_cont_pad[0].stick_x,
            os_cont_pad[0].stick_y,
            (os_cont_pad[0].button & 0x0200) ? '<' : ' ',
            " v^X"[os_cont_pad[0].button >> 10 & 3],
            (os_cont_pad[0].button & 0x0100) ? '>' : ' ',
            (os_cont_pad[0].button & 0x8000) ? 'A' : ' ',
            (os_cont_pad[0].button & 0x4000) ? 'B' : ' ',
            (os_cont_pad[0].button & 0x2000) ? 'Z' : ' ',
            (os_cont_pad[0].button & 0x1000) ? 'S' : ' ',
            (os_cont_pad[0].button & 0x0020) ? 'L' : ' ',
            (os_cont_pad[0].button & 0x0010) ? 'R' : ' ',
            (os_cont_pad[0].button & 0x0002) ? '<' : ' ',
            " v^X"[os_cont_pad[0].button >> 2 & 3],
            (os_cont_pad[0].button & 0x0001) ? '>' : ' ',
            (uint)((contdemo-contdemo_data)/4)
        );
        contdemo      += 4;
        contdemo_size -= 4;
        if (contdemo_size == 0) free(contdemo_data);
    }
#endif
}
