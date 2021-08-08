void lib_guNormalize(void)
{
    f32 x = __read_f32(a0);
    f32 y = __read_f32(a1);
    f32 z = __read_f32(a2);
    f32 d = 1 / sqrtf(x*x + y*y + z*z);
    __write_f32(a0, d*x);
    __write_f32(a1, d*y);
    __write_f32(a2, d*z);
}
