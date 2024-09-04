#define sats_over(slice)        (((slice) > +32767) ? +32767  : (slice))
#define sats_under(slice)       (((slice) < -32768) ? -32768  : (slice))
#define pack_signed(slice)      sats_over(sats_under(slice))
#define MultQ15(l, r) (((l)*(r) + 0x4000) >> 15)
static inline s32 mixer_macc(s32* Acc, s32* AdderStart, s32* AdderEnd, s32 Ramp)
{
	s32 volume = (*AdderEnd - *AdderStart) >> 3;
	*Acc = *AdderStart;
	*AdderEnd   = (s32)((((s64)(*AdderEnd) * (s64)Ramp) >> 16) & 0xFFFFFFFF);
	*AdderStart = (s32)((((s64)(*Acc)      * (s64)Ramp) >> 16) & 0xFFFFFFFF);
	return volume;
}

static void a_envmixer(u32 w0, u32 w1)
{
	uint  flag = w0 >> 16;
	void *addr = asp_addr(w1);
#ifdef ASP_MAIN1
	s16  *inp  = asp_s16(asp_dmemin);
	s16  *out  = asp_s16(asp_dmemout);
	s16  *aux0 = asp_s16(asp_aux_0);
	s16  *aux1 = asp_s16(asp_aux_1);
	s16  *aux2 = asp_s16(asp_aux_2);
#endif
#ifdef ASP_NAUDIO
	s16  *inp  = asp_s16(*asp_u16(asp_dmemin));
	s16  *out  = asp_s16(*asp_u16(asp_dmemout));
	s16  *aux0 = asp_s16(*asp_u16(asp_aux_0));
	s16  *aux1 = asp_s16(*asp_u16(asp_aux_1));
	s16  *aux2 = asp_s16(*asp_u16(asp_aux_2));
#endif
	s32   MainR;
	s32   MainL;
	s32   AuxR;
	s32   AuxL;
	s32   i1;
	u16   AuxIncRate = 1;
	s16   zero[8];
	s32   LVol, RVol;
	s32   LAcc, RAcc;
	s32   LTrg, RTrg;
	s16   Wet, Dry;
	u32   ptr = 0;
	s32   RRamp, LRamp;
	s32   LAdderStart, RAdderStart, LAdderEnd, RAdderEnd;
	s32   oMainR, oMainL, oAuxR, oAuxL;
	memset(zero, 0, sizeof(zero));
	if (flag & A_INIT)
	{
		LVol = asp_vol_l * (s32)asp_volrate_l;
		RVol = asp_vol_r * (s32)asp_volrate_r;
		Wet = (s16)asp_wet_gain;
		Dry = (s16)asp_dry_gain;
		LTrg = asp_voltgt_l << 16;
		RTrg = asp_voltgt_r << 16;
		LAdderStart = asp_vol_l << 16;
		RAdderStart = asp_vol_r << 16;
		LAdderEnd = LVol;
		RAdderEnd = RVol;
		RRamp = asp_volrate_r;
		LRamp = asp_volrate_l;
	}
	else
	{
		/* probably not the right address */
		wordswap(asp_s16(0xF90), addr, 0x50);
		Wet = *asp_s16(0xF90);
		Dry = *asp_s16(0xF92);
		LTrg = *asp_s16(0xF94) << 16 | *asp_u16(0xF96);
		RTrg = *asp_s16(0xF96) << 16 | *asp_u16(0xF98);
		LRamp = *asp_s16(0xF98) << 16 | *asp_u16(0xF9A);
		RRamp = *asp_s16(0xF9A) << 16 | *asp_u16(0xF9C);
		LAdderEnd = *asp_s16(0xF9C) << 16 | *asp_u16(0xF9E);
		RAdderEnd = *asp_s16(0xF9E) << 16 | *asp_u16(0xFA0);
		LAdderStart = *asp_s16(0xFA0) << 16 | *asp_u16(0xFA2);
		RAdderStart = *asp_s16(0xFA2) << 16 | *asp_u16(0xFA4);
	}
	if (!(flag & A_AUX))
	{
		AuxIncRate = 0;
		aux1 = aux2 = zero;
	}
	oMainL = MultQ15(Dry, (LTrg >> 16));
	oAuxL = MultQ15(Wet, (LTrg >> 16));
	oMainR = MultQ15(Dry, (RTrg >> 16));
	oAuxR = MultQ15(Wet, (RTrg >> 16));
	for (int y = 0; y < asp_count; y += 16)
	{
		if (LAdderStart != LTrg)
		{
			LVol = mixer_macc(&LAcc, &LAdderStart, &LAdderEnd, LRamp);
		}
		else
		{
			LAcc = LTrg;
			LVol = 0;
		}
		if (RAdderStart != RTrg)
		{
			RVol = mixer_macc(&RAcc, &RAdderStart, &RAdderEnd, RRamp);
		}
		else
		{
			RAcc = RTrg;
			RVol = 0;
		}
		for (int x = 0; x < 8; x++)
		{
			LAcc += LVol;
			if ((LVol <= 0 && LAcc < LTrg) || (LVol > 0 && LAcc > LTrg))
			{
				LAcc = LTrg;
				LAdderStart = LTrg;
				MainL = oMainL;
				AuxL = oAuxL;
			}
			else
			{
				MainL = MultQ15(Dry, ((s32)LAcc >> 16));
				AuxL = MultQ15(Wet, ((s32)LAcc >> 16));
			}

			RAcc += RVol;
			if ((RVol <= 0 && RAcc < RTrg) || (RVol > 0 && RAcc > RTrg))
			{
				RAcc = RTrg;
				RAdderStart = RTrg;
				MainR = oMainR;
				AuxR = oAuxR;
			}
			else
			{
				MainR = MultQ15(Dry, ((s32)RAcc >> 16));
				AuxR = MultQ15(Wet, ((s32)RAcc >> 16));
			}
			i1 = inp[ptr];
			out[ptr] = pack_signed(out[ptr]+MultQ15((s16)i1, (s16)MainR));
			aux0[ptr] = pack_signed(aux0[ptr]+MultQ15((s16)i1, (s16)MainL));
			if (AuxIncRate)
			{
				aux1[ptr] = pack_signed(aux1[ptr]+MultQ15((s16)i1, (s16)AuxR));
				aux2[ptr] = pack_signed(aux2[ptr]+MultQ15((s16)i1, (s16)AuxL));
			}
			ptr++;
		}
	}
	*asp_s16(0xF90) = Wet;
	*asp_s16(0xF92) = Dry;
	*asp_s16(0xF94) = LTrg >> 16;
	*asp_u16(0xF96) = LTrg;
	*asp_s16(0xF96) = RTrg >> 16;
	*asp_u16(0xF98) = RTrg;
	*asp_s16(0xF98) = LRamp >> 16;
	*asp_u16(0xF9A) = LRamp;
	*asp_s16(0xF9A) = RRamp >> 16;
	*asp_u16(0xF9C) = RRamp;
	*asp_s16(0xF9C) = LAdderEnd >> 16;
	*asp_u16(0xF9E) = LAdderEnd;
	*asp_s16(0xF9E) = RAdderEnd >> 16;
	*asp_u16(0xFA0) = RAdderEnd;
	*asp_s16(0xFA0) = LAdderStart >> 16;
	*asp_u16(0xFA2) = LAdderStart;
	*asp_s16(0xFA2) = RAdderStart >> 16;
	*asp_u16(0xFA4) = RAdderStart;
	wordswap(addr, asp_s16(0xF90), 80);
}
