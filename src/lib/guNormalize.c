void lib_guNormalize(void)
{
    f32 x = __read_f32(a0.i[IX]);
    f32 y = __read_f32(a1.i[IX]);
    f32 z = __read_f32(a2.i[IX]);
    f32 d = 1.0F / sqrtf(x*x + y*y + z*z);
    __write_f32(a0.i[IX], d*x);
    __write_f32(a1.i[IX], d*y);
    __write_f32(a2.i[IX], d*z);
}
