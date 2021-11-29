static void rsp_gfxtask(void *ucode, void *data)
{
    gsp_update(ucode, data);
}

static void rsp_audtask(void *data, u32 size)
{
    asp_update(data, size);
}

static void rsp_init(void)
{
    gsp_init();
}

static void rsp_exit(void)
{
    gsp_exit();
}
