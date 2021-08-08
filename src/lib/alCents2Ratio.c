void lib_alCents2Ratio(void)
{
    s32 c = a0;
    f32 x = 1;
    f32 m;
    if (c < 0)
    {
        m = 0.9994225441F;
        c = -c;
    }
    else
    {
        m = 1.00057779F;
    }
    do
    {
        if (c & 1)
        {
            x *= m;
        }
        m *= m;
        c >>= 1;
    }
    while (c != 0);
    ARG_F(v0) = x;
}
