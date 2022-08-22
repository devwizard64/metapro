#ifdef DEBUG
static void gdp_g_rdppipesync(UNUSED u32 w0, UNUSED u32 w1)
{
}
#else
#define gdp_g_rdppipesync       NULL
#endif
