void lib_osPiStartDma(void)
{
    PTR src  = a3.iu[IX];
    PTR dst  = __read_s32(sp.i[IX] + 0x0010);
    u32 size = __read_u32(sp.i[IX] + 0x0014);
    PTR mq   = __read_s32(sp.i[IX] + 0x0018);
    __dma(__tlb(dst), src, size);
    a0.i[IX] = mq;
    a1.i[IX] = 0;
    a2.i[IX] = OS_MESG_NOBLOCK;
    lib_osSendMesg();
}
