static Thread gsp_thread = NULL;
#ifndef LLE
static Thread asp_thread = NULL;
#endif
static Handle gsp_start  = 0;
#ifndef LLE
static Handle asp_start  = 0;
#endif
static Handle gsp_end    = 0;
#ifndef LLE
static Handle asp_end    = 0;
#endif
static PTR    gsp_ucode  = NULLPTR;
static u32   *gsp_data   = NULL;
#ifndef LLE
static u32   *asp_data   = NULL;
static u32    asp_size   = 0;
#endif
static bool   rsp_update = TRUE;

static void gsp_main(UNUSED void *arg)
{
    gsp_init();
    do
    {
        svcSignalEvent(gsp_end);
        svcWaitSynchronization(gsp_start, U64_MAX);
        svcClearEvent(gsp_start);
        if (gsp_data != NULL) gsp_update(gsp_ucode, gsp_data);
    }
    while (rsp_update);
    gsp_exit();
}

#ifndef LLE
static void asp_main(UNUSED void *arg)
{
    do
    {
        svcSignalEvent(asp_end);
        svcWaitSynchronization(asp_start, U64_MAX);
        svcClearEvent(asp_start);
        if (asp_data != NULL) asp_update(asp_data, asp_size);
    }
    while (rsp_update);
}
#endif

static void rsp_init(void)
{
    svcCreateEvent(&gsp_start, RESET_ONESHOT);
    svcCreateEvent(&gsp_end,   RESET_ONESHOT);
#ifndef LLE
    svcCreateEvent(&asp_start, RESET_ONESHOT);
    svcCreateEvent(&asp_end,   RESET_ONESHOT);
#endif
    gsp_thread = threadCreate(gsp_main, NULL, 0x2000, 0x3F, -1, 1);
#ifndef LLE
    asp_thread = threadCreate(asp_main, NULL, 0x2000, 0x3F, -1, 1);
#endif
}

static void rsp_exit(void)
{
    rsp_update = FALSE;
    if (gsp_thread != NULL)
    {
        gsp_data = NULL;
        svcSignalEvent(gsp_start);
        threadJoin(gsp_thread, U64_MAX);
        svcCloseHandle(gsp_start);
    }
#ifndef LLE
    if (asp_thread != NULL)
    {
        asp_data = NULL;
        svcSignalEvent(asp_start);
        threadJoin(asp_thread, U64_MAX);
        svcCloseHandle(asp_start);
    }
#endif
}

void rsp_gfxtask(PTR ucode, void *data)
{
    gsp_ucode = ucode;
    gsp_data  = data;
    svcWaitSynchronization(gsp_end, U64_MAX);
    svcClearEvent(gsp_end);
    svcSignalEvent(gsp_start);
}

#ifndef LLE
void rsp_audtask(void *data, u32 size)
{
    asp_data = data;
    asp_size = size;
    svcWaitSynchronization(asp_end, U64_MAX);
    svcClearEvent(asp_end);
    svcSignalEvent(asp_start);
}
#endif
