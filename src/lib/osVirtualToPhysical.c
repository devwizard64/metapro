void lib_osVirtualToPhysical(void)
{
    PTR addr = a0;
    if (addr >= 0x80000000)
    {
        v0 = addr & 0x1FFFFFFF;
    }
#ifdef APP_UNSM
    else if (addr >= 0x04000000)
    {
        v0 = addr - 0x04000000 + 0x00390000;
    }
#endif
    else
    {
        wdebug("vaddr 0x%08" FMT_X "U\n", addr);
    }
}
