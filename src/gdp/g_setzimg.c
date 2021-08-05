#ifdef __DEBUG__
static void gdp_g_setzimg(unused u32 w0, unused u32 w1)
{
}
#else
#define gdp_g_setzimg           NULL
#endif
