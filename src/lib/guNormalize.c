void lib_guNormalize(void)
{
    float x = __read_f32(a0);
    float y = __read_f32(a1);
    float z = __read_f32(a2);
    float d = 1 / sqrtf(x*x + y*y + z*z);
    __write_f32(a0, d*x);
    __write_f32(a1, d*y);
    __write_f32(a2, d*z);
}
