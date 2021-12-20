static void asp_a_mixer(u32 w0, u32 w1)
{
    if (asp_count > 0)
    {
        s16 *dmemin  = asp_s16(asp_sample + (w1 >> 16         ));
        s16 *dmemout = asp_s16(asp_sample + (w1 >>  0 & 0xFFFF));
        s32  gain    = (s16)(w0 >>  0         );
        uint count   = (asp_count + 0x1F) & ~0x1F;
        do
        {
            s32 x = *dmemout + ((*dmemin*gain) >> 15);
            if (x < -0x8000) x = -0x8000;
            if (x > +0x7FFF) x = +0x7FFF;
            *dmemout = x;
            dmemin  += 1;
            dmemout += 1;
            count   -= 2;
        }
        while (count > 0);
    }
}
