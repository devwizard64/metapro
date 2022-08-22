#ifdef DEBUG
static void gdp_g_rdploadsync(UNUSED u32 w0, UNUSED u32 w1)
{
}
#else
#define gdp_g_rdploadsync       NULL
#endif
