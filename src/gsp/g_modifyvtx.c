static void gsp_g_modifyvtx(u32 w0, u32 w1)
{
    uint offset = w0 >> 16 & 0xFF;
    uint index  = w0 >>  0 & 0xFFFF;
    struct vtxf *vf = &gsp_vtxf_buf[index];
    switch (offset)
    {
        case G_MWO_POINT_RGBA:
            vf->shade[0] = w1 >> 24;
            vf->shade[1] = w1 >> 16;
            vf->shade[2] = w1 >>  8;
            vf->shade[3] = w1 >>  0;
            break;
        case G_MWO_POINT_ST:
            vf->s = (s16)(w1 >> 16);
            vf->t = (s16)(w1 >>  0);
            if (gsp_texture_filter != GL_NEAREST)
            {
                vf->s += 32*0.5F;
                vf->t += 32*0.5F;
            }
            break;
        default:
            edebug("modifyvtx %02X\n", offset);
            break;
    }
}
