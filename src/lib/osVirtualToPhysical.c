void lib_osVirtualToPhysical(void)
{
    if (a0.iu[IX] >= 0x80000000)
    {
        v0.i[IX] = a0.iu[IX] & 0x1FFFFFFF;
    }
#ifdef APP_UNSM
    else if (a0.iu[IX] >= 0x04000000)
    {
        v0.i[IX] = a0.iu[IX] - 0x04000000 + 0x00390000;
    }
#endif
    else
    {
        wdebug("vaddr 0x%08" FMT_X "U\n", a0.iu[IX]);
    }
}
