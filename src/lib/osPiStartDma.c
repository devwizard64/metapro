void lib_osPiStartDma(void)
{
    PTR src  = a3;
    PTR dst  = __read_s32(sp + 0x0010);
    u32 size = __read_u32(sp + 0x0014);
    PTR mq   = __read_s32(sp + 0x0018);
    __dma(__tlb(dst), src, size);
    a0 = mq;
    a1 = 0;
    a2 = OS_MESG_NOBLOCK;
    lib_osSendMesg();
}
