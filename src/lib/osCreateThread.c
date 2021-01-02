void lib_osCreateThread(void)
{
    thread_init(
        a0.i[IX],
        a1.i[IX],
        a2.i[IX],
        a3.i[IX],
        __read_s32(sp.i[IX] + 0x0010),
        __read_s32(sp.i[IX] + 0x0014)
    );
}
