static void gsp_g_dl(u32 w0, u32 w1)
{
    if ((w0 >> 16 & 0xFF) == G_DL_PUSH)
    {
        gsp_dl_index++;
    }
    gsp_dl_stack[gsp_dl_index] = gsp_addr(w1);
}
