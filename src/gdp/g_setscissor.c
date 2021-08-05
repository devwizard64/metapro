static void gdp_g_setscissor(u32 w0, u32 w1)
{
    gdp_scissor_l = w0 >> 12 & 0x0FFF;
    gdp_scissor_t = w0 >>  0 & 0x0FFF;
    gdp_scissor_r = w1 >> 12 & 0x0FFF;
    gdp_scissor_b = w1 >>  0 & 0x0FFF;
    gsp_change |= CHANGE_SCISSOR;
}
