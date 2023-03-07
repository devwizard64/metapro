static void a_segment(UNUSED u32 w0, u32 w1)
{
    asp_addr_table[w1 >> 24 & 0xF] = &cpu_dram[w1 & 0xFFFFFF];
}
