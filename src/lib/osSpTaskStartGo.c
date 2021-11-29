/* header says it returns a s32, but the asm doesn't set v0 */
void lib_osSpTaskStartGo(void)
{
    PTR   task  = a0;
    u32   type  = __read_u32(task+0x00);
    void *ucode = __tlb(__read_s32(task+0x10));
    void *data  = __tlb(__read_s32(task+0x30));
    u32   size  = __read_u32(task+0x34);
    if (type == M_GFXTASK)
    {
        rsp_gfxtask(ucode, data);
        os_event(&os_event_table[OS_EVENT_DP]);
    }
    else
    {
        rsp_audtask(data, size);
    }
    os_event(&os_event_table[OS_EVENT_SP]);
}
