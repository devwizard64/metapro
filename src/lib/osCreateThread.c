void lib_osCreateThread(void)
{
    thread_init(
        a0, a1, a2, a3,
        __read_s32(sp + 0x0010),
        __read_s32(sp + 0x0014)
    );
}
