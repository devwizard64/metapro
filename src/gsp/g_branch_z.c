static void g_branch_z(UNUSED u32 w0, UNUSED u32 w1)
{
	/* todo: branchz */
	gsp_dl_stack[gsp_dl_index] = gsp_addr(gdp_texrect[2]);
}
