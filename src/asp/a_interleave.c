static void asp_a_interleave(unused u32 w0, unused u32 w1)
{
    if (asp_count > 0)
    {
        s16 *dmemout = asp_s16(asp_dmemout);
    #ifdef ASP_MAIN1
        s16 *inl = asp_s16(asp_sample + (w1 >> 16         ));
        s16 *inr = asp_s16(asp_sample + (w1 >>  0 & 0xFFFF));
    #endif
    #ifdef ASP_NAUDIO
        s16 *inl = asp_s16(asp_aux_0);
        s16 *inr = asp_s16(asp_dmemout);
    #endif
        uint count = (asp_count + 0x0F) & ~0x0F;
        do
        {
            dmemout[0] = inl[0];
            dmemout[1] = inr[0];
            dmemout += 2;
            inl     += 1;
            inr     += 1;
            count   -= 2;
        }
        while (count > 0);
    }
}
