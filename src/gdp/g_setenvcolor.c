static void g_setenvcolor(UNUSED u32 w0, u32 w1)
{
	gdp_env[0] = w1 >> 24;
	gdp_env[1] = w1 >> 16;
	gdp_env[2] = w1 >>  8;
	gdp_env[3] = w1 >>  0;
}
