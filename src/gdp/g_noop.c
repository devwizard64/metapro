#if defined(APP_UNKT) && defined(DEBUG)
static void g_noop(UNUSED u32 w0, UNUSED u32 w1)
{
}
#else
#define g_noop                  NULL
#endif
