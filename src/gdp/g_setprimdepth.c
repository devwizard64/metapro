#ifndef APP_UNSM
static void gdp_g_setprimdepth(UNUSED u32 w0, UNUSED u32 w1)
{
    /* todo: primdepth (?) */
}
#else
#define gdp_g_setprimdepth      NULL
#endif
