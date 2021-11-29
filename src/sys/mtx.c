void mtx_read(f32 *dst, const s16 *src)
{
    uint cnt = 4*4;
    do
    {
    #ifdef __EB__
        dst[0x00] = (1.0F/0x10000) * (s32)(src[0x00] << 16 | (u16)src[0x10]);
        dst[0x01] = (1.0F/0x10000) * (s32)(src[0x01] << 16 | (u16)src[0x11]);
    #else
        dst[0x00] = (1.0F/0x10000) * (s32)(src[0x01] << 16 | (u16)src[0x11]);
        dst[0x01] = (1.0F/0x10000) * (s32)(src[0x00] << 16 | (u16)src[0x10]);
    #endif
        dst += 0x02;
        src += 0x02;
        cnt -= 0x02;
    }
    while (cnt > 0);
}

void mtx_write(s16 *dst, const f32 *src)
{
    uint cnt = 4*4;
    do
    {
        s32 a = 0x10000 * src[0x00];
        s32 b = 0x10000 * src[0x01];
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
        dst += 0x02;
        src += 0x02;
        cnt -= 0x02;
    }
    while (cnt > 0);
}

void mtxf_cat(f32 mf[4][4], f32 a[4][4], f32 b[4][4])
{
    uint y;
    for (y = 0; y < 4; y++)
    {
        uint x;
        for (x = 0; x < 4; x++)
        {
            mf[y][x] =
                a[y][0]*b[0][x] + a[y][1]*b[1][x] +
                a[y][2]*b[2][x] + a[y][3]*b[3][x];
        }
    }
}

void mtxf_identity(f32 mf[4][4])
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

void mtxf_ortho(
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

#ifndef APP_UNK4
static void mtxf_perspective(
    f32 mf[4][4], float fovy, float aspect, float n, float f
)
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
#endif

#ifdef APP_UNKT
static void mtxf_lookat(
    f32 mf[4][4],
    float ex, float ey, float ez,
    float ax, float ay, float az,
    float ux, float uy, float uz
)
{
    float d;
    float lx;
    float ly;
    float lz;
    float rx;
    float ry;
    float rz;
    lx = ax - ex;
    ly = ay - ey;
    lz = az - ez;
    d = -1 / sqrtf(lx*lx + ly*ly + lz*lz);
    lx *= d;
    ly *= d;
    lz *= d;
    rx = uy*lz - uz*ly;
    ry = uz*lx - ux*lz;
    rz = ux*ly - uy*lx;
    d = 1 / sqrtf (rx*rx + ry*ry + rz*rz);
    rx *= d;
    ry *= d;
    rz *= d;
    ux = ly*rz - lz*ry;
    uy = lz*rx - lx*rz;
    uz = lx*ry - ly*rx;
    d = 1 / sqrtf (ux*ux + uy*uy + uz*uz);
    ux *= d;
    uy *= d;
    uz *= d;
    mf[0][0] = rx;
    mf[0][1] = ux;
    mf[0][2] = lx;
    mf[0][3] = 0;
    mf[1][0] = ry;
    mf[1][1] = uy;
    mf[1][2] = ly;
    mf[1][3] = 0;
    mf[2][0] = rz;
    mf[2][1] = uz;
    mf[2][2] = lz;
    mf[2][3] = 0;
    mf[3][0] = -(ex*rx + ey*ry + ez*rz);
    mf[3][1] = -(ex*ux + ey*uy + ez*uz);
    mf[3][2] = -(ex*lx + ey*ly + ez*lz);
    mf[3][3] = 1;
}
#endif

#ifndef APP_UNK4
static void mtxf_translate(f32 mf[4][4], float x, float y, float z)
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
    mf[3][0] = x;
    mf[3][1] = y;
    mf[3][2] = z;
    mf[3][3] = 1;
}

static void mtxf_rotate(f32 mf[4][4], float a, float x, float y, float z)
{
    float xx;
    float yy;
    float zz;
    float s;
    float c;
    float xyc;
    float yzc;
    float zxc;
    xx = x*x;
    yy = y*y;
    zz = z*z;
    s = 1 / sqrtf(xx + yy + zz);
    x *= s;
    y *= s;
    z *= s;
    a *= (float)(M_PI/180);
    s = sinf(a);
    c = cosf(a);
    xyc = x*y*(1-c);
    yzc = y*z*(1-c);
    zxc = z*x*(1-c);
    mf[0][0] = (1-xx)*c + xx;
    mf[0][1] = xyc + z*s;
    mf[0][2] = zxc - y*s;
    mf[0][3] = 0;
    mf[1][0] = xyc - z*s;
    mf[1][1] = (1-yy)*c + yy;
    mf[1][2] = yzc + x*s;
    mf[1][3] = 0;
    mf[2][0] = zxc + y*s;
    mf[2][1] = yzc - x*s;
    mf[2][2] = (1-zz)*c + zz;
    mf[2][3] = 0;
    mf[3][0] = 0;
    mf[3][1] = 0;
    mf[3][2] = 0;
    mf[3][3] = 1;
}

static void mtxf_scale(f32 mf[4][4], float x, float y, float z)
{
    mf[0][0] = x;
    mf[0][1] = 0;
    mf[0][2] = 0;
    mf[0][3] = 0;
    mf[1][0] = 0;
    mf[1][1] = y;
    mf[1][2] = 0;
    mf[1][3] = 0;
    mf[2][0] = 0;
    mf[2][1] = 0;
    mf[2][2] = z;
    mf[2][3] = 0;
    mf[3][0] = 0;
    mf[3][1] = 0;
    mf[3][2] = 0;
    mf[3][3] = 1;
}
#endif
