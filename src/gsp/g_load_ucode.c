static void gsp_g_load_ucode(unused u32 w0, u32 w1)
{
    gsp_flush_triangles();
    gsp_start(gsp_addr(w1), gsp_dl_stack[gsp_dl_index]);
}