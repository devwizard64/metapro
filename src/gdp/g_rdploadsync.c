#ifdef DEBUG
static void gdp_g_rdploadsync(unused u32 w0, unused u32 w1)
{
}
#else
#define gdp_g_rdploadsync       NULL
#endif
