static void gdp_set_rm(u32 mode)
{
	if (AC) glEnable(GL_ALPHA_TEST);
	else    glDisable(GL_ALPHA_TEST);
	if (BL) glEnable(GL_BLEND);
	else    glDisable(GL_BLEND);
	if (ZW) gsp_polyfmt |=  (1 << 11);
	else    gsp_polyfmt &= ~(1 << 11);
	if (DE) gsp_polyfmt |=  (1 << 14);
	else    gsp_polyfmt &= ~(1 << 14);
}
