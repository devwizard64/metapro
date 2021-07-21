#ifdef _EB
#define __read_str(dst, src) strcpy(dst, __tlb(src))
#define __write_str(dst, src) strcpy(__tlb(dst), src)
#else
#define __read_str(dst, src)    \
{                               \
    char *_dst = dst;           \
    u32   _src = src;           \
    char  _c;                   \
    do                          \
    {                           \
        _c = __read_u8(_src++); \
        *_dst++ = _c;           \
    }                           \
    while (_c != 0x00);         \
}
#define __write_str(dst, src)   \
{                               \
    u32   _dst = dst;           \
    char *_src = src;           \
    char  _c;                   \
    do                          \
    {                           \
        _c = *_src++;           \
        __write_u8(_dst++, _c); \
    }                           \
    while (_c != 0x00);         \
}
#endif

void lib_sprintf(void)
{
    char  buf_dst[0x100];
    char  buf_fmt[0x100];
    s32   buf_arg[6];
    char *dst;
    char *fmt;
    s32  *arg;
    uint  i;
    __read_str(buf_fmt, a1.i[IX]);
    buf_arg[0] = a2.iu[IX];
    buf_arg[1] = a3.iu[IX];
    for (i = 2; i < lenof(buf_arg); i++)
    {
        buf_arg[i] = __read_s32(sp.i[IX] + 0x0008 + 4*i);
    }
    dst = buf_dst;
    fmt = buf_fmt;
    arg = buf_arg;
    while (*fmt != 0)
    {
        if (*fmt == '%')
        {
            switch (*++fmt)
            {
                case 'd':   dst += sprintf(dst, "%" FMT_d, *arg++); break;
                case 'x':   dst += sprintf(dst, "%" FMT_x, *arg++); break;
                default:    edebug("invalid fmt %c\n", *fmt);       break;
            }
            fmt++;
        }
        else
        {
            *dst++ = *fmt++;
        }
    }
    *dst = 0x00;
    __write_str(a0.i[IX], buf_dst);
    v0.ll = (s32)(dst-buf_dst);
}
