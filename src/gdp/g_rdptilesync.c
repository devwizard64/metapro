#ifdef DEBUG
static void g_rdptilesync(UNUSED u32 w0, UNUSED u32 w1)
{
}
#else
#define g_rdptilesync           NULL
#endif
