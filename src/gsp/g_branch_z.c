static void gsp_g_branch_z(unused u32 w0, unused u32 w1)
{
    /* todo: branchz */
    gsp_dl_stack[gsp_dl_index] = gsp_addr(gsp_rdphalf_1);
}
