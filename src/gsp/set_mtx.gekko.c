#define TRANSPOSE(i)            \
{                               \
    mt[i][0] = mf[0][i];        \
    mt[i][1] = mf[1][i];        \
    mt[i][2] = mf[2][i];        \
    mt[i][3] = mf[3][i];        \
}

static void gsp_set_mp(f32 mf[4][4])
{
    Mtx44 mt;
    TRANSPOSE(0);
    TRANSPOSE(1);
    TRANSPOSE(2);
    TRANSPOSE(3);
    if (gsp_decal) mt[2][3]--;
    GX_LoadProjectionMtx(mt, mt[3][3] != 0 ? GX_ORTHOGRAPHIC : GX_PERSPECTIVE);
}

static void gsp_set_mm(f32 mf[4][4])
{
    Mtx mt;
    TRANSPOSE(0);
    TRANSPOSE(1);
    TRANSPOSE(2);
    GX_LoadPosMtxImm(mt, GX_PNMTX0);
}

#undef TRANSPOSE
