#ifndef APP_UNSM
static void g_rdpsetothermode(u32 w0, u32 w1)
{
    gdp_othermode_h = w0;
    gdp_othermode_l = w1;
    gdp_flush_rm();
}
#else
#define g_rdpsetothermode       NULL
#endif
