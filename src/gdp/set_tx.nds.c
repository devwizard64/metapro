static void gdp_set_tx(GDP_TX tx)
{
	if (tx != GDP_TX_NULL)
	{
		glEnable(GL_TEXTURE_2D);
		gsp_polyfmt &= ~(3 << 4);
		gsp_polyfmt |= tx;
	}
	else
	{
		glDisable(GL_TEXTURE_2D);
	}
}
