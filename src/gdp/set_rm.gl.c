static void gdp_set_rm(u32 mode)
{
    if (ZR) glEnable(GL_DEPTH_TEST);
    else    glDisable(GL_DEPTH_TEST);
    glDepthMask(ZW);
    if (AC) glEnable(GL_ALPHA_TEST);
    else    glDisable(GL_ALPHA_TEST);
    if (BL) glEnable(GL_BLEND);
    else    glDisable(GL_BLEND);
    if (DE) glPolygonOffset(-1, -2);
    else    glPolygonOffset(0, 0);
}
