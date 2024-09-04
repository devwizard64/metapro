static void a_clearbuff(u32 w0, u32 w1)
{
	uint count = w1 & 0xFFFF;
	if (count > 0)
	{
		uint dmem = w0 & 0xFFFF;
		memset(asp_s16(asp_sample + dmem), 0, (count+15) & ~15);
	}
}
