#ifdef __DEBUG__
static void gdp_g_rdpfullsync(unused u32 w0, unused u32 w1)
{
}
#else
#define gdp_g_rdpfullsync       NULL
#endif
