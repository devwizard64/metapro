static void rsp_init(void)
{
    gsp_init();
}

static void rsp_exit(void)
{
    gsp_exit();
}

void rsp_gfxtask(PTR ucode, void *data)
{
    gsp_update(ucode, data);
}

#ifndef LLE
void rsp_audtask(void *data, u32 size)
{
    asp_update(data, size);
}
#endif
