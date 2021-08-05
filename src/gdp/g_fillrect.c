static void gdp_g_fillrect(u32 w0, u32 w1)
{
    if (gdp_fill != 0xFFFC)
    {
        uint xh;
        uint yh;
        uint xl;
        uint yl;
        uint r;
        uint g;
        uint b;
        uint i;
        u8  t[3];
        if (gdp_rect == 0)
        {
            gsp_mtxf_projection++;
            gsp_mtxf_modelview++;
            mtxf_identity(MM);
            gsp_change |=
                CHANGE_MTXF_MODELVIEW | CHANGE_CULL |
                CHANGE_RENDERMODE | CHANGE_TEXTURE_ENABLED;
        }
        if (gdp_rect != 2)
        {
            gdp_rect = 2;
            mtxf_ortho(MP, 0, 320, 240, 0, -1, 1);
            gsp_change |= CHANGE_MTXF_PROJECTION;
        }
        xh = w0 >> 14 & 0x03FF;
        yh = w0 >>  2 & 0x03FF;
        xl = w1 >> 14 & 0x03FF;
        yl = w1 >>  2 & 0x03FF;
        r = RGBA16_R(gdp_fill);
        g = RGBA16_G(gdp_fill);
        b = RGBA16_B(gdp_fill);
        if (gdp_cycle)
        {
            xh++;
            yh++;
        }
        for (i = 0; i < 4; i++)
        {
            struct vtx  *v  = &gsp_vtx_buf[i];
            struct vtxf *vf = &gsp_vtxf_buf[i];
            v->x = i == 0 || i == 3 ? xl : xh;
            v->y = i == 2 || i == 3 ? yl : yh;
            v->z = 0;
            vf->shade[0] = r;
            vf->shade[1] = g;
            vf->shade[2] = b;
            vf->shade[3] = 0xFF;
        }
        t[0] = 0;
        t[1] = 1;
        t[2] = 2;
        gdp_write_triangle(t);
        t[0] = 0;
        t[1] = 2;
        t[2] = 3;
        gdp_write_triangle(t);
    }
}
