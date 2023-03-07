static void g_special_1(UNUSED u32 w0, UNUSED u32 w1)
{
#ifdef APP_UNK4
    mtx_read(&MM[0][0], gsp_mtx);
#endif
}
