void lib_osAiSetFrequency(void)
{
    s32 dacrate = 48681812/(float)a0 + 0.5F;
    s32 freq = 48681812/dacrate;
    v0 = audio_freq(freq);
}
