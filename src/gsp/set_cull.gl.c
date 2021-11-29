static void gsp_set_cull(u32 mode)
{
    switch (mode)
    {
        case 0:
            glDisable(GL_CULL_FACE);
            break;
        case G_CULL_FRONT:
            glEnable(GL_CULL_FACE);
            glCullFace(GL_FRONT);
            break;
        case G_CULL_BACK:
            glEnable(GL_CULL_FACE);
            glCullFace(GL_BACK);
            break;
        case G_CULL_BOTH:
            glEnable(GL_CULL_FACE);
            glCullFace(GL_FRONT_AND_BACK);
            break;
    }
}
