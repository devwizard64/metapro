static void gsp_set_mp(f32 mf[4][4])
{
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(&mf[0][0]);
}

static void gsp_set_mm(f32 mf[4][4])
{
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(&mf[0][0]);
}
