static void a_loadadpcm(u32 w0, u32 w1)
{
	wordswap(asp_s16(asp_adpcm), asp_addr(w1), w0 & 0xFFFF);
}
