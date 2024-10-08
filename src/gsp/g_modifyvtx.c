static void g_modifyvtx(u32 w0, u32 w1)
{
	uint offset = w0 >> 16 & 0xFF;
	uint index  = w0 >>  1 & 0x7FFF;
	VTXF *vf = &gsp_vtxf_buf[index];
	switch (offset)
	{
	/* ? */
	case 0x00:
		break;
	case G_MWO_POINT_RGBA:
		vf->shade[0] = w1 >> 24;
		vf->shade[1] = w1 >> 16;
		vf->shade[2] = w1 >>  8;
		vf->shade[3] = w1 >>  0;
		break;
	case G_MWO_POINT_ST:
		vf->s = (s16)(w1 >> 16);
		vf->t = (s16)(w1 >>  0);
		if (gdp_tf != GDP_TF_POINT)
		{
			vf->s += 32*0.5F;
			vf->t += 32*0.5F;
		}
		break;
	default:
		wdebug("modifyvtx %02X\n", offset);
		break;
	}
}
