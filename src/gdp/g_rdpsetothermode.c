#if 0
static void gdp_g_rdpsetothermode(u32 w0, u32 w1)
{
    gdp_othermode_h = w0;
    gdp_othermode_l = w1;
    gsp_flush_rm();
}
#else
#define gdp_g_rdpsetothermode   NULL
#endif
