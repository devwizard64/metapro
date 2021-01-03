static void gsp_g_popmtx(unused u32 w0, unused u32 w1)
{
#ifdef GSP_F3D
    gsp_mtxf_modelview--;
#endif
#ifdef GSP_F3DEX2
    gsp_mtxf_modelview -= w1 / 0x40;
#endif
    gsp_change |= CHANGE_MTXF_MODELVIEW;
    gsp_light_new = true;
}
