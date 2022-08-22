#include "types.h"

void mtx_read(f32 *dst, const s16 *src)
{
    uint cnt = 4*4;
    do
    {
#ifdef __EB__
        dst[0] = (1.0F/0x10000) * (s32)(src[0x00] << 16 | (u16)src[0x10]);
        dst[1] = (1.0F/0x10000) * (s32)(src[0x01] << 16 | (u16)src[0x11]);
#else
        dst[0] = (1.0F/0x10000) * (s32)(src[0x01] << 16 | (u16)src[0x11]);
        dst[1] = (1.0F/0x10000) * (s32)(src[0x00] << 16 | (u16)src[0x10]);
#endif
        dst += 2;
        src += 2;
        cnt -= 2;
    }
    while (cnt > 0);
}

void mtx_write(s16 *dst, const f32 *src)
{
    uint cnt = 4*4;
    do
    {
        s32 a = 0x10000 * src[0];
        s32 b = 0x10000 * src[1];
#ifdef __EB__
        dst[0x00] = a >> 16;
        dst[0x01] = b >> 16;
        dst[0x10] = a >>  0;
        dst[0x11] = b >>  0;
#else
        dst[0x00] = b >> 16;
        dst[0x01] = a >> 16;
        dst[0x10] = b >>  0;
        dst[0x11] = a >>  0;
#endif
        dst += 2;
        src += 2;
        cnt -= 2;
    }
    while (cnt > 0);
}

void mtx_cat(f32 mf[4][4], f32 a[4][4], f32 b[4][4])
{
    int y;
    int x;
    for (y = 0; y < 4; y++)
    {
        for (x = 0; x < 4; x++)
        {
            mf[y][x] =
                a[y][0]*b[0][x] + a[y][1]*b[1][x] +
                a[y][2]*b[2][x] + a[y][3]*b[3][x];
        }
    }
}

void mtx_identity(f32 mf[4][4])
{
    mf[0][0] = 1;
    mf[0][1] = 0;
    mf[0][2] = 0;
    mf[0][3] = 0;
    mf[1][0] = 0;
    mf[1][1] = 1;
    mf[1][2] = 0;
    mf[1][3] = 0;
    mf[2][0] = 0;
    mf[2][1] = 0;
    mf[2][2] = 1;
    mf[2][3] = 0;
    mf[3][0] = 0;
    mf[3][1] = 0;
    mf[3][2] = 0;
    mf[3][3] = 1;
}

void mtx_ortho(
    f32 mf[4][4], float l, float r, float b, float t, float n, float f
)
{
    mf[0][0] = 2 / (r-l);
    mf[0][1] = 0;
    mf[0][2] = 0;
    mf[0][3] = 0;
    mf[1][0] = 0;
    mf[1][1] = 2 / (t-b);
    mf[1][2] = 0;
    mf[1][3] = 0;
    mf[2][0] = 0;
    mf[2][1] = 0;
#ifdef GEKKO
    mf[2][2] = 1 / (n-f);
#else
    mf[2][2] = 2 / (n-f);
#endif
    mf[2][3] = 0;
    mf[3][0] = (l+r) / (l-r);
    mf[3][1] = (b+t) / (b-t);
#ifdef GEKKO
    mf[3][2] = (  f) / (n-f);
#else
    mf[3][2] = (n+f) / (n-f);
#endif
    mf[3][3] = 1;
}

void mtx_perspective(f32 mf[4][4], float fovy, float aspect, float n, float f)
{
    float x;
    fovy *= (float)(M_PI/360);
    x = cosf(fovy) / sinf(fovy);
    mf[0][0] = x/aspect;
    mf[0][1] = 0;
    mf[0][2] = 0;
    mf[0][3] = 0;
    mf[1][0] = 0;
    mf[1][1] = x;
    mf[1][2] = 0;
    mf[1][3] = 0;
    x = 1 / (n-f);
    mf[2][0] = 0;
    mf[2][1] = 0;
#ifdef GEKKO
    mf[2][2] = (n  )*x;
#else
    mf[2][2] = (n+f)*x;
#endif
    mf[2][3] = -1;
    mf[3][0] = 0;
    mf[3][1] = 0;
#ifdef GEKKO
    mf[3][2] =   n*f*x;
#else
    mf[3][2] = 2*n*f*x;
#endif
    mf[3][3] = 0;
}
