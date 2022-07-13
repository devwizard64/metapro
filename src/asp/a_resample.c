static void asp_a_resample(u32 w0, u32 w1)
{
#ifdef ASP_MAIN1
    uint flag    = w0 >> 16;
    int  pitch   = w0 & 0xFFFF;
#endif
#ifdef ASP_NAUDIO
    uint flag    = w0 >> 30;
    int  pitch   = w0 >> 14 & 0xFFFF;
#endif
    s32 *addr    = asp_addr(w1);
#ifdef ASP_MAIN1
    s16 *dmemout = asp_s16(asp_dmemout);
    s16 *dmemin  = asp_s16(asp_dmemin);
#endif
#ifdef ASP_NAUDIO
    s16 *dmemout = asp_s16(asp_dmemout + (w0 >> 2 & 0x0FFF));
    s16 *dmemin  = (w0 & 3) ? asp_s16(0x0660) : asp_s16(asp_dmemin);
#endif
    uint count   = asp_count;
    int pos;
    pitch *= 2;
    dmemin -= 16/2;
    if (flag & A_INIT)
    {
        memset(dmemin, 0, 16);
        pos = 0;
    }
    else
    {
        wordswap(dmemin, addr, 16);
        pos = addr[5] & 0xFFFF;
    }
    count += 32;
    do
    {
        uint p = pos >> 16 & 0xFFFF;
        int  f = pos >>  0 & 0xFFFF;
        int  x0 = dmemin[p+0];
        int  x1 = dmemin[p+1];
        *dmemout = x0 + (x1-x0)*f/0x10000;
        pos += pitch;
        dmemout += 1;
        count   -= 2;
    }
    while (count > 0);
    pos -= 16*pitch;
    dmemin += pos >> 16;
    wordswap(addr, dmemin, 16);
    addr[5] = pos;
}
