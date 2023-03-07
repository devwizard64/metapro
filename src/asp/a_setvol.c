static void a_setvol(u32 w0, u32 w1)
{
    uint flag = w0 >> 16;
#ifdef ASP_MAIN1
    if (flag & A_AUX)
    {
        asp_dry_gain = w0;
        asp_wet_gain = w1;
    }
    else
    {
        if (flag & A_VOL)
        {
            if (flag & A_LEFT)
            {
                asp_vol_l = w0;
            }
            else
            {
                asp_vol_r = w0;
            }
        }
        else
        {
            if (flag & A_LEFT)
            {
                asp_voltgt_l  = w0;
                asp_volrate_l = w1;
            }
            else
            {
                asp_voltgt_r  = w0;
                asp_volrate_r = w1;
            }
        }
    }
#endif
#ifdef ASP_NAUDIO
    if (flag & A_VOL)
    {
        if (flag & A_LEFT)
        {
            asp_vol_l    = w0;
            asp_dry_gain = w1 >> 16;
            asp_wet_gain = w1;
        }
        else
        {
            asp_voltgt_r  = w0;
            asp_volrate_r = w1;
        }
    }
    else
    {
        asp_voltgt_l  = w0;
        asp_volrate_l = w1;
    }
#endif
}
