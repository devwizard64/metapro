static void g_movemem(u32 w0, u32 w1)
{
#ifdef GSP_F3D
    static void *const g_movemem_table[] =
    {
        /* 0x80 G_MV_VIEWPORT */ &gsp_viewport,
        /* 0x82 G_MV_LOOKATY  */ &gsp_light_buf[0],
        /* 0x84 G_MV_LOOKATX  */ &gsp_light_buf[1],
        /* 0x86 G_MV_L0       */ &gsp_light_buf[2],
        /* 0x88 G_MV_L1       */ &gsp_light_buf[3],
        /* 0x8A G_MV_L2       */ &gsp_light_buf[4],
        /* 0x8C G_MV_L3       */ &gsp_light_buf[5],
        /* 0x8E G_MV_L4       */ &gsp_light_buf[6],
        /* 0x90 G_MV_L5       */ &gsp_light_buf[7],
        /* 0x92 G_MV_L6       */ &gsp_light_buf[8],
        /* 0x94 G_MV_L7       */ &gsp_light_buf[9],
        /* 0x96 G_MV_TXTATT   */ NULL,
    };
    uint index = w0 >> 16 & 0xFF;
    uint size  = w0 >>  0 & 0xFFFF;
    if (index >= G_MV_VIEWPORT && index < G_MV_TXTATT+2)
    {
        void *dst = g_movemem_table[(index-0x80) / 2];
        if (dst != NULL)
        {
            memcpy(dst, gsp_addr(w1), size);
            if (index == G_MV_VIEWPORT)
            {
                gsp_flush_vp();
            }
            else if (index >= G_MV_LOOKATY && index < G_MV_L7+2)
            {
                gsp_new_light = TRUE;
            }
        }
    }
#endif
#ifdef GSP_F3DEX2
    static void *const g_movemem_table[] =
    {
        /* 0x08 G_MV_VIEWPORT */ &gsp_viewport,
        /* 0x0A G_MV_LIGHT    */ gsp_light_buf,
        /* 0x0C G_MV_POINT    */ NULL,
        /* 0x0E G_MV_MATRIX   */ NULL,
    };
    uint size   = w0 >> 16 & 0xFF;
    uint offset = w0 >>  8 & 0xFF;
    uint index  = w0 >>  0 & 0xFF;
    if (index >= G_MV_VIEWPORT && index < G_MV_MATRIX+2)
    {
        u64 *dst = g_movemem_table[(index-0x08) / 2];
        if (dst != NULL)
        {
            memcpy(&dst[offset], gsp_addr(w1), size+8);
            switch (index)
            {
                case G_MV_VIEWPORT:
                    gsp_flush_vp();
                    break;
                case G_MV_LIGHT:
                    gsp_new_light = TRUE;
                    break;
            }
        }
    }
#endif
}
