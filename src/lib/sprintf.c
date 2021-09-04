void lib_sprintf(void)
{
    char  buf_dst[0x100];
    char  buf_fmt[0x100];
    s32   buf_arg[6];
    char *dst;
    char *fmt;
    s32  *arg;
    uint  i;
    __read_str(buf_fmt, a1);
    buf_arg[0] = a2;
    buf_arg[1] = a3;
    for (i = 2; i < lenof(buf_arg); i++)
    {
        buf_arg[i] = __read_s32(sp + 0x0008 + 4*i);
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
    __write_str(a0, buf_dst);
    v0 = dst-buf_dst;
}
