static void asp_a_setloop(UNUSED u32 w0, u32 w1)
{
    asp_loop = asp_addr(w1);
}
