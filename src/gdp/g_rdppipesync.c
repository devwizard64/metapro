#ifdef DEBUG
static void gdp_g_rdppipesync(unused u32 w0, unused u32 w1)
{
}
#else
#define gdp_g_rdppipesync       NULL
#endif
