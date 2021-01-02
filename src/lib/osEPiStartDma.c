void lib_osEPiStartDma(void)
{
    s32 mq   = __read_s32(a1.i[IX]+0x04);
    s32 dst  = __read_s32(a1.i[IX]+0x08);
    u32 src  = __read_u32(a1.i[IX]+0x0C);
    u32 size = __read_u32(a1.i[IX]+0x10);
    __dma(__tlb(dst), src & 0x0FFFFFFF, size);
    a0.i[IX] = mq;
    a1.i[IX] = 0;
    a2.i[IX] = OS_MESG_NOBLOCK;
    lib_osSendMesg();
}
