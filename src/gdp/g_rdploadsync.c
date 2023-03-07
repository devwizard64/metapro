#ifdef DEBUG
static void g_rdploadsync(UNUSED u32 w0, UNUSED u32 w1)
{
}
#else
#define g_rdploadsync           NULL
#endif
