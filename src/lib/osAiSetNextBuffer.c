void lib_osAiSetNextBuffer(void)
{
    audio_update(__tlb(a0.i[IX]), a1.i[IX]);
    v0.ll = (s32)0;
}
