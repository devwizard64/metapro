static void gsp_g_cleargeometrymode(unused u32 w0, u32 w1)
{
    gsp_geometry_mode &= ~w1;
    if (w1 & G_CULL_BOTH)
    {
        gsp_change |= CHANGE_CULL;
    }
}
