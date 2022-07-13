static void rsp_init(void)
{
#if defined(GSP_F3D) || defined(GSP_F3DEX2)
    gsp_init();
#endif
}

static void rsp_exit(void)
{
#if defined(GSP_F3D) || defined(GSP_F3DEX2)
    gsp_exit();
#endif
}

#if defined(GSP_F3D) || defined(GSP_F3DEX2)
void rsp_gfxtask(PTR ucode, void *data)
{
    gsp_update(ucode, data);
}
#endif

#ifndef LLE
void rsp_audtask(void *data, u32 size)
{
    asp_update(data, size);
}
#endif
