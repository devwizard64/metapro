static Thread gsp_thread = NULL;
static Thread asp_thread = NULL;
static Handle gsp_start  = 0;
static Handle asp_start  = 0;
static Handle gsp_end    = 0;
static Handle asp_end    = 0;
static void  *gsp_ucode  = NULL;
static u32   *gsp_data   = NULL;
static u32   *asp_data   = NULL;
static u32    asp_size   = 0;
static bool   rsp_update = true;

static void gsp_main(unused void *arg)
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

static void asp_main(unused void *arg)
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

static void rsp_gfxtask(void *ucode, void *data)
{
    gsp_ucode = ucode;
    gsp_data  = data;
    svcWaitSynchronization(gsp_end, U64_MAX);
    svcClearEvent(gsp_end);
    svcSignalEvent(gsp_start);
}

static void rsp_audtask(void *data, u32 size)
{
    asp_data = data;
    asp_size = size;
    svcWaitSynchronization(asp_end, U64_MAX);
    svcClearEvent(asp_end);
    svcSignalEvent(asp_start);
}

static void rsp_init(void)
{
    svcCreateEvent(&gsp_start, RESET_ONESHOT);
    svcCreateEvent(&gsp_end,   RESET_ONESHOT);
    svcCreateEvent(&asp_start, RESET_ONESHOT);
    svcCreateEvent(&asp_end,   RESET_ONESHOT);
    gsp_thread = threadCreate(gsp_main, NULL, 0x2000, 0x3F, -1, 1);
    asp_thread = threadCreate(asp_main, NULL, 0x2000, 0x3F, -1, 1);
}

static void rsp_exit(void)
{
    rsp_update = false;
    if (gsp_thread != NULL)
    {
        gsp_data = NULL;
        svcSignalEvent(gsp_start);
        threadJoin(gsp_thread, U64_MAX);
        svcCloseHandle(gsp_start);
    }
    if (asp_thread != NULL)
    {
        asp_data = NULL;
        svcSignalEvent(asp_start);
        threadJoin(asp_thread, U64_MAX);
        svcCloseHandle(asp_start);
    }
}
