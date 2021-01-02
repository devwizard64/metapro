/* header says it returns a s32, but the asm doesn't set v0 */
void lib_osSpTaskStartGo(void)
{
    s32   task  = a0.i[IX];
#ifndef APP_SEQ
    u32   type  = __read_u32(task+0x00);
    void *ucode = __tlb(__read_u32(task+0x10));
#endif
    void *data  = __tlb(__read_s32(task+0x30));
    u32   size  = __read_u32(task+0x34);
#ifndef APP_SEQ
    if (type == M_GFXTASK)
    {
    #ifdef _3DS
        lib_gsp_ucode = ucode;
        lib_gsp_data  = data;
        svcWaitSynchronization(lib_gsp_end, U64_MAX);
        svcClearEvent(lib_gsp_end);
        svcSignalEvent(lib_gsp_start);
    #else
        gsp_update(ucode, data);
    #ifdef _NATIVE
        SDL_GL_SwapWindow(lib_window);
    #endif
    #ifdef _GCN
        lib_video_draw = 1;
    #endif
    #endif
        lib_event(&lib_event_table[OS_EVENT_DP]);
    }
    else
#endif
    {
    #ifdef _3DS
        lib_asp_data = data;
        lib_asp_size = size;
        svcWaitSynchronization(lib_asp_end, U64_MAX);
        svcClearEvent(lib_asp_end);
        svcSignalEvent(lib_asp_start);
    #else
    #if defined(APP_UNSM) /* && defined(APP_E0) */
        asp_update(data, size);
    #else
        (void)size;
    #endif
    #endif
    }
    lib_event(&lib_event_table[OS_EVENT_SP]);
}
