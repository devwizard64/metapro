#if defined(APP_UNKT) && defined(DEBUG)
static void gdp_g_noop(unused u32 w0, unused u32 w1)
{
}
#else
#define gdp_g_noop              NULL
#endif
