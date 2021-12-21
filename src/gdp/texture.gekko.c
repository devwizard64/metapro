#define i (w*(ty+iy)+(tx+ix))

static void *gdp_texture_rgba16(TXTARG *arg, const u8 *src, uint w, uint h)
{
    void *buf;
    u8   *dst;
    uint ty;
    uint tx;
    uint iy;
    uint ix;
    arg->fmt = GX_TF_RGBA8;
    buf = dst = memalign(0x20, 4*w*h);
    for (ty = 0; ty < h; ty += 4)
    {
        for (tx = 0; tx < w; tx += 4)
        {
            for (iy = 0; iy < 4; iy++)
            {
                for (ix = 0; ix < 4; ix++)
                {
                    uint x = src[2*i+0] << 8 | src[2*i+1];
                    dst[0x00] = RGBA16_A(x);
                    dst[0x01] = RGBA16_R(x);
                    dst[0x20] = RGBA16_G(x);
                    dst[0x21] = RGBA16_B(x);
                    dst += 2;
                }
            }
            dst += 0x20;
        }
    }
    DCFlushRange(buf, 4*w*h);
    return buf;
}

#ifndef APP_UNSM
static void *gdp_texture_rgba32(TXTARG *arg, const u8 *src, uint w, uint h)
{
    void *buf;
    u8   *dst;
    uint ty;
    uint tx;
    uint iy;
    uint ix;
    arg->fmt = GX_TF_RGBA8;
    buf = dst = memalign(0x20, 4*w*h);
    for (ty = 0; ty < h; ty += 4)
    {
        for (tx = 0; tx < w; tx += 4)
        {
            for (iy = 0; iy < 4; iy++)
            {
                for (ix = 0; ix < 4; ix++)
                {
                    dst[0x00] = src[4*i+3];
                    dst[0x01] = src[4*i+2];
                    dst[0x20] = src[4*i+1];
                    dst[0x21] = src[4*i+0];
                    dst += 2;
                }
            }
            dst += 0x20;
        }
    }
    DCFlushRange(buf, 4*w*h);
    return buf;
}
#else
#define gdp_texture_rgba32 NULL
#endif

#define gdp_texture_yuv16  NULL

static void *gdp_texture_ia4(TXTARG *arg, const u8 *src, uint w, uint h)
{
    void *buf;
    u8   *dst;
    uint ty;
    uint tx;
    uint iy;
    uint ix;
    arg->fmt = GX_TF_IA4;
    buf = dst = memalign(0x20, w*h);
    for (ty = 0; ty < h; ty += 4)
    {
        for (tx = 0; tx < w; tx += 8)
        {
            for (iy = 0; iy < 4; iy++)
            {
                for (ix = 0; ix < 8; ix += 2)
                {
                    dst[0] = IA4_AIH(src[i/2]);
                    dst[1] = IA4_AIL(src[i/2]);
                    dst += 2;
                }
            }
        }
    }
    DCFlushRange(buf, w*h);
    return buf;
}

static void *gdp_texture_ia8(TXTARG *arg, const u8 *src, uint w, uint h)
{
    void *buf;
    u8   *dst;
    uint ty;
    uint tx;
    uint iy;
    uint ix;
    arg->fmt = GX_TF_IA4;
    buf = dst = memalign(0x20, w*h);
    for (ty = 0; ty < h; ty += 4)
    {
        for (tx = 0; tx < w; tx += 8)
        {
            for (iy = 0; iy < 4; iy++)
            {
                for (ix = 0; ix < 8; ix++)
                {
                    dst[0] = src[i] << 4 | src[i] >> 4;
                    dst += 1;
                }
            }
        }
    }
    DCFlushRange(buf, w*h);
    return buf;
}

#ifdef APP_UNSM
static void *gdp_texture_ia8_face(TXTARG *arg, const u8 *src, uint w, uint h)
{
    void *buf;
    u8   *dst;
    uint ty;
    uint tx;
    uint iy;
    uint ix;
    arg->fmt = GX_TF_IA4;
    buf = dst = memalign(0x20, w*h);
    for (ty = 0; ty < h; ty += 4)
    {
        for (tx = 0; tx < w; tx += 8)
        {
            for (iy = 0; iy < 4; iy++)
            {
                for (ix = 0; ix < 8; ix++)
                {
                    dst[0] = 0x0F | src[i];
                    dst += 1;
                }
            }
        }
    }
    DCFlushRange(buf, w*h);
    return buf;
}
#endif

static void *gdp_texture_ia16(TXTARG *arg, const u8 *src, uint w, uint h)
{
    void *buf;
    u8   *dst;
    uint ty;
    uint tx;
    uint iy;
    uint ix;
    arg->fmt = GX_TF_IA8;
    buf = dst = memalign(0x20, 2*w*h);
    for (ty = 0; ty < h; ty += 4)
    {
        for (tx = 0; tx < w; tx += 4)
        {
            for (iy = 0; iy < 4; iy++)
            {
                for (ix = 0; ix < 4; ix++)
                {
                    dst[0] = src[2*i+1];
                    dst[1] = src[2*i+0];
                    dst += 2;
                }
            }
        }
    }
    DCFlushRange(buf, 2*w*h);
    return buf;
}

#ifndef APP_UNSM
static void *gdp_texture_i4(TXTARG *arg, const u8 *src, uint w, uint h)
{
    void *buf;
    u8   *dst;
    uint ty;
    uint tx;
    uint iy;
    uint ix;
    arg->fmt = GX_TF_IA4;
    buf = dst = memalign(0x20, w*h);
    for (ty = 0; ty < h; ty += 4)
    {
        for (tx = 0; tx < w; tx += 8)
        {
            for (iy = 0; iy < 4; iy++)
            {
                for (ix = 0; ix < 8; ix += 2)
                {
                    dst[0] = I4_IH(src[i/2]);
                    dst[1] = I4_IL(src[i/2]);
                    dst += 2;
                }
            }
        }
    }
    DCFlushRange(buf, w*h);
    return buf;
}

static void *gdp_texture_i8(TXTARG *arg, const u8 *src, uint w, uint h)
{
    void *buf;
    u8   *dst;
    uint ty;
    uint tx;
    uint iy;
    uint ix;
    arg->fmt = GX_TF_IA8;
    buf = dst = memalign(0x20, 2*w*h);
    for (ty = 0; ty < h; ty += 4)
    {
        for (tx = 0; tx < w; tx += 4)
        {
            for (iy = 0; iy < 4; iy++)
            {
                for (ix = 0; ix < 4; ix++)
                {
                    dst[0] = dst[1] = src[i];
                    dst += 2;
                }
            }
        }
    }
    DCFlushRange(buf, 2*w*h);
    return buf;
}
#else
#define gdp_texture_i4     NULL
#define gdp_texture_i8     NULL
#endif

#undef i
