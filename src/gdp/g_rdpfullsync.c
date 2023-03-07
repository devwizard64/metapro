#ifdef DEBUG
static void g_rdpfullsync(UNUSED u32 w0, UNUSED u32 w1)
{
}
#else
#define g_rdpfullsync           NULL
#endif
