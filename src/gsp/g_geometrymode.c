static void gsp_g_geometrymode(unused u32 w0, unused u32 w1)
{
    gsp_geometry_mode &= w0;
    gsp_geometry_mode |= w1;
    if ((~w0 | w1) & G_CULL_BOTH)
    {
        gsp_change |= CHANGE_CULL;
    }
}
