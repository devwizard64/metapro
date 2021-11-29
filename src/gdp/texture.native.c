static void *gdp_texture_rgba16(GDP_FMT *fmt, const u8 *src, uint w, uint h)
{
    void *buf;
    u8   *dst;
    uint  len;
    *fmt = GL_RGBA8;
    buf = dst = malloc(4*w*h);
    len = w*h;
    do
    {
        uint x = src[0] << 8 | src[1];
        dst[0] = RGBA16_R(x);
        dst[1] = RGBA16_G(x);
        dst[2] = RGBA16_B(x);
        dst[3] = RGBA16_A(x);
        dst += 4;
        src += 2;
        len -= 1;
    }
    while (len > 0);
    return buf;
}

#ifndef APP_UNSM
static void *gdp_texture_rgba32(GDP_FMT *fmt, const u8 *src, uint w, uint h)
{
    *fmt = GL_RGBA8;
    return memcpy(malloc(4*w*h), src, 4*w*h);
}
#else
#define gdp_texture_rgba32 NULL
#endif

#define gdp_texture_yuv16  NULL

static void *gdp_texture_ia4(GDP_FMT *fmt, const u8 *src, uint w, uint h)
{
    void *buf;
    u8   *dst;
    uint  len;
    *fmt = GL_RGBA8;
    buf = dst = malloc(4*w*h);
    len = w*h/2;
    do
    {
        dst[0] = dst[1] = dst[2] = IA4_IH(src[0]);
        dst[3]                   = IA4_AH(src[0]);
        dst[4] = dst[5] = dst[6] = IA4_IL(src[0]);
        dst[7]                   = IA4_AL(src[0]);
        dst += 8;
        src += 1;
        len -= 1;
    }
    while (len > 0);
    return buf;
}

static void *gdp_texture_ia8(GDP_FMT *fmt, const u8 *src, uint w, uint h)
{
    void *buf;
    u8   *dst;
    uint  len;
    *fmt = GL_RGBA8;
    buf = dst = malloc(4*w*h);
    len = w*h;
    do
    {
        dst[0] = dst[1] = dst[2] = IA8_I(src[0]);
        dst[3]                   = IA8_A(src[0]);
        dst += 4;
        src += 1;
        len -= 1;
    }
    while (len > 0);
    return buf;
}

#ifdef APP_UNSM
static void *gdp_texture_ia8_face(GDP_FMT *fmt, const u8 *src, uint w, uint h)
{
    void *buf;
    u8   *dst;
    uint  len;
    *fmt = GL_RGBA8;
    buf = dst = malloc(4*w*h);
    len = w*h;
    do
    {
        dst[0] = dst[1] = dst[2] = 0xFF;
        dst[3]                   = IA8_I(src[0]);
        dst += 4;
        src += 1;
        len -= 1;
    }
    while (len > 0);
    return buf;
}
#endif

static void *gdp_texture_ia16(GDP_FMT *fmt, const u8 *src, uint w, uint h)
{
    void *buf;
    u8   *dst;
    uint  len;
    *fmt = GL_RGBA8;
    buf = dst = malloc(4*w*h);
    len = w*h;
    do
    {
        dst[0] = dst[1] = dst[2] = src[0];
        dst[3]                   = src[1];
        dst += 4;
        src += 2;
        len -= 1;
    }
    while (len > 0);
    return buf;
}

#ifndef APP_UNSM
static void *gdp_texture_i4(GDP_FMT *fmt, const u8 *src, uint w, uint h)
{
    void *buf;
    u8   *dst;
    uint  len;
    *fmt = GL_RGBA8;
    buf = dst = malloc(4*w*h);
    len = w*h/2;
    do
    {
        dst[0] = dst[1] = dst[2] = dst[3] = I4_IH(src[0]);
        dst[4] = dst[5] = dst[6] = dst[7] = I4_IL(src[0]);
        dst += 8;
        src += 1;
        len -= 1;
    }
    while (len > 0);
    return buf;
}

static void *gdp_texture_i8(GDP_FMT *fmt, const u8 *src, uint w, uint h)
{
    void *buf;
    u8   *dst;
    uint  len;
    *fmt = GL_RGBA8;
    buf = dst = malloc(4*w*h);
    len = w*h;
    do
    {
        dst[0] = dst[1] = dst[2] = dst[3] = src[0];
        dst += 4;
        src += 1;
        len -= 1;
    }
    while (len > 0);
    return buf;
}
#else
#define gdp_texture_i4     NULL
#define gdp_texture_i8     NULL
#endif
