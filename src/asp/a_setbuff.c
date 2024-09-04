static void a_setbuff(u32 w0, u32 w1)
{
	uint flag    = w0 >> 16;
	uint dmemin  = w0 & 0xFFFF;
	uint dmemout = w1 >> 16;
	uint count   = w1 & 0xFFFF;
	dmemin  += asp_sample;
	dmemout += asp_sample;
	if (flag & A_AUX)
	{
		count += asp_sample;
		asp_aux_0 = dmemin;
		asp_aux_1 = dmemout;
		asp_aux_2 = count;
	}
	else
	{
		asp_dmemin  = dmemin;
		asp_dmemout = dmemout;
		asp_count   = count;
	}
}
