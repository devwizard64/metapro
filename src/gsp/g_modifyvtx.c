static void gsp_g_modifyvtx(u32 w0, u32 w1)
{
    uint offset = w0 >> 16 & 0xFF;
    uint index  = w0 >>  0 & 0xFFFF;
    struct vtxf_t *vtxf = &gsp_vtxf_buf[index];
    switch (offset)
    {
        case G_MWO_POINT_RGBA:
            vtxf->shade[0] = w1 >> 24;
            vtxf->shade[1] = w1 >> 16;
            vtxf->shade[2] = w1 >>  8;
            vtxf->shade[3] = w1 >>  0;
            break;
        case G_MWO_POINT_ST:
            vtxf->u = (s16)(w1 >> 16);
            vtxf->v = (s16)(w1 >>  0);
            if (gsp_texture_filter != GL_NEAREST)
            {
                vtxf->u += 32.0F*0.5F;
                vtxf->v += 32.0F*0.5F;
            }
            break;
        default:
        #ifdef _DEBUG
            fprintf(stderr, "error: modifyvtx %02X\n", offset);
            exit(EXIT_FAILURE);
        #endif
            break;
    }
}
