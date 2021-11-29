static void gsp_set_cull(u32 mode)
{
    switch (mode)
    {
        case 0:             GX_SetCullMode(GX_CULL_NONE);   break;
        case G_CULL_FRONT:  GX_SetCullMode(GX_CULL_BACK);   break;
        case G_CULL_BACK:   GX_SetCullMode(GX_CULL_FRONT);  break;
        case G_CULL_BOTH:   GX_SetCullMode(GX_CULL_ALL);    break;
    }
}
