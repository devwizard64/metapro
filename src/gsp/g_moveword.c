static void gsp_g_moveword(u32 w0, u32 w1)
{
#ifdef GSP_F3D
    uint index  = w0 >>  0 & 0xFF;
    uint offset = w0 >>  8 & 0xFFFF;
#endif
#ifdef GSP_F3DEX2
    uint index  = w0 >> 16 & 0xFF;
    uint offset = w0 >>  0 & 0xFFFF;
#endif
    switch (index)
    {
#ifdef APP_UNK4
        case G_MW_MATRIX:
            *(u32 *)((u8 *)gsp_mtx + offset) = w1;
            break;
#endif
        case G_MW_NUMLIGHT:
#ifdef GSP_F3D
            gsp_light_no = 1 + (w1-0x80000000)/0x20;
#endif
#ifdef GSP_F3DEX2
            gsp_light_no = 1 + w1/0x18;
#endif
            break;
        case G_MW_CLIP:
            break;
        case G_MW_SEGMENT:
            gsp_addr_table[offset/4] = &cpu_dram[w1 & 0xFFFFFF];
            break;
        case G_MW_FOG:
            gsp_fog_m = w1 >> 16;
            gsp_fog_o = w1 >>  0;
            gsp_new_fog = TRUE;
            break;
#ifndef APP_UNSM
        case G_MW_LIGHTCOL:
            *(u32 *)((u8 *)gsp_light_buf + offset) = w1;
            gsp_new_light = TRUE;
            break;
#endif
#ifndef GSP_F3D_20D
        case G_MW_PERSPNORM:
            break;
#endif
        default:
            wdebug("moveword of %02X:%04X\n", index, offset);
            break;
    }
}
