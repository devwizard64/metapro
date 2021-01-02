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
    #ifdef _DEBUG
        fprintf(stderr, "error: vaddr 0x%08XU\n", a0.iu[IX]);
    #endif
    }
}
