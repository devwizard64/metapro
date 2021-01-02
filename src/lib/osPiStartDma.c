void lib_osPiStartDma(void)
{
    u32 src  = a3.iu[IX];
    s32 dst  = __read_s32(sp.i[IX] + 0x0010);
    u32 size = __read_u32(sp.i[IX] + 0x0014);
    s32 mq   = __read_s32(sp.i[IX] + 0x0018);
    __dma(__tlb(dst), src, size);
    a0.i[IX] = mq;
    a1.i[IX] = 0;
    a2.i[IX] = OS_MESG_NOBLOCK;
    lib_osSendMesg();
}
