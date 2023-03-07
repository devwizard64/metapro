#ifdef DEBUG
static void g_rdppipesync(UNUSED u32 w0, UNUSED u32 w1)
{
}
#else
#define g_rdppipesync           NULL
#endif
