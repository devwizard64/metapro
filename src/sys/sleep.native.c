static u64 sleep_timer = 0;

static u64 sleep_time(void)
{
#ifdef WIN32
    LARGE_INTEGER count;
    LARGE_INTEGER freq;
    QueryPerformanceCounter(&count);
    QueryPerformanceFrequency(&freq);
    return 1000000000*count.QuadPart/freq.QuadPart;
#else
    struct timespec timespec;
    clock_gettime(CLOCK_MONOTONIC, &timespec);
    return 1000000000*timespec.tv_sec + timespec.tv_nsec;
#endif
}

static void sleep_frame(void)
{
    u64 time = sleep_time();
    if (time-sleep_timer > 50000000) sleep_timer = time;
    sleep_timer += 16666667;
    while (sleep_timer > time)
    {
        SDL_Delay(1);
        time = sleep_time();
    }
}
