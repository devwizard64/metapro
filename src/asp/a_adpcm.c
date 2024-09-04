static void a_adpcm(u32 w0, u32 w1)
{
#ifdef ASP_MAIN1
	uint  flag    = w0 >> 16;
#endif
#ifdef ASP_NAUDIO
	uint  flag    = w0 >> 28;
#endif
	void *addr    = asp_addr(w1);
#ifdef ASP_MAIN1
	uint  dmemin  = asp_dmemin;
	s16  *dmemout = asp_s16(asp_dmemout);
#endif
#ifdef ASP_NAUDIO
	uint  dmemin  = asp_sample + (w0 >> 12 & 0xF);
	s16  *dmemout = asp_s16(asp_sample + (w0 >> 0 & 0xFFF));
#endif
	s16  *adpcm   = asp_s16(asp_adpcm);
	int   count   = asp_count;
	if (flag & A_INIT) memset(dmemout, 0, 32);
	else wordswap(dmemout, (flag & A_LOOP) ? asp_loop : addr, 32);
	dmemout += 32/2;
	while (count > 0)
	{
		int i;
		int m1;
		int m0;
		int l1;
		int l0;
		int d;
		uint shift = 11 + (*asp_u8(dmemin) >> 4);
		i  = *asp_u8(dmemin++) << 4 & 0xF0;
		m1 = adpcm[i+0];
		m0 = adpcm[i+8];
		l1 = dmemout[-2];
		l0 = dmemout[-1];
		d = 0;
		for (i = 0; i < 16; i++)
		{
			int s;
			if (i & 1) d <<= 4;
			else d = *asp_u8(dmemin++) << 24;
			s = (((d >> 28) << shift) + l1*m1 + l0*m0) >> 11;
			if (s < -0x8000) s = -0x8000;
			if (s > +0x7FFF) s = +0x7FFF;
			*dmemout++ = s;
			l1 = l0;
			l0 = s;
		}
		count -= 32;
	}
	dmemout -= 32/2;
	wordswap(addr, dmemout, 32);
}
