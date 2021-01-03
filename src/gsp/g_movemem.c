static void gsp_g_movemem(u32 w0, u32 w1)
{
#ifdef GSP_F3D
    uint index = w0 >> 16 & 0xFF;
    uint size  = w0 >>  0 & 0xFFFF;
    if (index >= G_MV_VIEWPORT && index < G_MV_TXTATT+2)
    {
        void *dst = gsp_movemem_table[(index-0x80) / 2];
        if (dst != NULL)
        {
            memcpy(dst, gsp_addr(w1), size);
            if (index == G_MV_VIEWPORT)
            {
                gsp_change |= CHANGE_VIEWPORT;
            }
            else if (index >= G_MV_LOOKATY && index < G_MV_L7+2)
            {
                if (index < G_MV_LOOKATX+2)
                {
                    gsp_lookat = true;
                }
                gsp_light_new = true;
            }
        }
    }
#endif
#ifdef GSP_F3DEX2
    uint size   = w0 >> 16 & 0xFF;
    uint offset = w0 >>  8 & 0xFF;
    uint index  = w0 >>  0 & 0xFF;
    if (index >= G_MV_VIEWPORT && index < G_MV_MATRIX+2)
    {
        u64 *dst = gsp_movemem_table[(index-0x08) / 2];
        if (dst != NULL)
        {
            memcpy(&dst[offset], gsp_addr(w1), size+8);
            switch (index)
            {
                case G_MV_VIEWPORT:
                    gsp_change |= CHANGE_VIEWPORT;
                    break;
                case G_MV_LIGHT:
                    if (offset < G_MVO_L0)
                    {
                        gsp_lookat = true;
                    }
                    gsp_light_new = true;
                    break;
            }
        }
    }
#endif
}
