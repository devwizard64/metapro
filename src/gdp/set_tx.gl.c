static void gdp_set_tx(GDP_TX tx)
{
	if (tx != GDP_TX_NULL)
	{
		glEnable(GL_TEXTURE_2D);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, tx);
	}
	else
	{
		glDisable(GL_TEXTURE_2D);
	}
}
