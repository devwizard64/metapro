void lib___ll_lshift(void)
{
    u64 a = (u64)a0.iu[IX] << 32 | (u64)a1.iu[IX];
    u64 b = (u64)a2.iu[IX] << 32 | (u64)a3.iu[IX];
    u64 x = a << b;
    v0.i[IX] = x >> 32;
    v1.i[IX] = x >>  0;
}
