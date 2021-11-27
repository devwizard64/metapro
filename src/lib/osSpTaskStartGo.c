/* header says it returns a s32, but the asm doesn't set v0 */
void lib_osSpTaskStartGo(void)
{
    PTR   task  = a0;
#ifndef APP_SEQ
    u32   type  = __read_u32(task+0x00);
    void *ucode = __tlb(__read_s32(task+0x10));
#endif
    void *data  = __tlb(__read_s32(task+0x30));
    u32   size  = __read_u32(task+0x34);
#ifndef APP_SEQ
    if (type == M_GFXTASK)
    {
    #ifdef __3DS__
        lib_gsp_ucode = ucode;
        lib_gsp_data  = data;
        svcWaitSynchronization(lib_gsp_end, U64_MAX);
        svcClearEvent(lib_gsp_end);
        svcSignalEvent(lib_gsp_start);
    #else
        gsp_update(ucode, data);
    #ifdef __NATIVE__
        SDL_GL_SwapWindow(lib_window);
    #endif
    #ifdef GEKKO
        GX_SetZMode(GX_TRUE, GX_ALWAYS, GX_TRUE);
        GX_SetColorUpdate(GX_TRUE);
        GX_CopyDisp(lib_framebuffer, GX_TRUE);
        GX_Flush();
    #endif
    #endif
        lib_event(&lib_event_table[OS_EVENT_DP]);
    }
    else
#endif
    {
    #if defined(APP_UNSM) /* && defined(APP_E0) */
    #ifdef __3DS__
        lib_asp_data = data;
        lib_asp_size = size;
        svcWaitSynchronization(lib_asp_end, U64_MAX);
        svcClearEvent(lib_asp_end);
        svcSignalEvent(lib_asp_start);
    #else
        asp_update(data, size);
    #endif
    #else
        (void)size;
    #endif
    }
    lib_event(&lib_event_table[OS_EVENT_SP]);
}
