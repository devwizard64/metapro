static void gsp_g_special_1(unused u32 w0, unused u32 w1)
{
#ifdef APP_UNK4
    mtx_read(&MM[0][0], gsp_mtx);
#endif
}
