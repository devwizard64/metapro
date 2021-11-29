static void mtx_cvt(int *dst, const f32 *src)
{
    uint i;
    for (i = 0; i < 16; i++) dst[i] = floattof32(src[i]);
}

static void gsp_set_mp(f32 mf[4][4])
{
    m4x4 m;
    mtx_cvt(&m.m[0], &mf[0][0]);
    glMatrixMode(GL_PROJECTION);
    glLoadMatrix4x4(&m);
}

static void gsp_set_mm(f32 mf[4][4])
{
    m4x4 m;
    mtx_cvt(&m.m[0], &mf[0][0]);
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrix4x4(&m);
    glScalef32(1 << 24, 1 << 24, 1 << 24);
}
