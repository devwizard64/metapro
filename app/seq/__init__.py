patch = None

entry = 0x80246000
bss   = [0x8033A580, 0x0002CEE0]
sp    = 0x80200600
main  = 0x80246DF8
reg = (
	1 <<  2 | # v0
	1 <<  3 | # v1
	1 <<  4 | # a0
	1 <<  5 | # a1
	1 <<  6 | # a2
	1 <<  7 | # a3
	1 << 29 | # sp
	1 << 32 | # f0
	1 << 38 | # f12
	1 << 39   # f14
)

header = (
	"#define EEPROM                  0\n"
	"#define AUDIO_FREQ              32000\n"
	"#define ASP_MAIN\n"
	"#define ASP_MAIN1\n"
	"#define APP_SEQ\n"
	"\n"
	"#define __osExceptionPreamble   0x80327640\n"
	"\n"
	"#define _AudioctlSegmentRomStart 0x57B720\n"
	"#define _AudiotblSegmentRomStart 0x593560\n"
	"#define _AudioseqSegmentRomStart 0x7B0860\n"
	"#define _AudiobnkSegmentRomStart 0x7CC620\n"
	"\n"
	"#define OS_PRIORITY_VIMGR       254\n"
	"#define OS_PRIORITY_PIMGR       150\n"
	"\n"
	"#define osSpTaskStart(tp) \\\n"
	"{ \\\n"
	"\tosSpTaskLoad((tp)); \\\n"
	"\tosSpTaskStartGo((tp)); \\\n"
	"}\n"
	"\n"
	"#define osCreateMesgQueue(mq, msg, count) \\\n"
	"\t{a0 = mq; a1 = msg; a2 = count; lib_osCreateMesgQueue()}\n"
	"#define osSetEventMesg(e, mq, m) \\\n"
	"\t{a0 = e; a1 = mq; a2 = m; lib_osSetEventMesg();}\n"
	"#define osViSetEvent(mq, msg, retraceCount) \\\n"
	"\t{a0 = mq; a1 = msg; a2 = retraceCount; lib_osViSetEvent();}\n"
	"#define osCreateThread(t, id, entry, arg, sp_, pri) \\\n"
	"{ \\\n"
	"\ta0 = t; a1 = id; a2 = entry; a3 = arg; \\\n"
	"\t*cpu_s32(sp+0x10) = sp_; \\\n"
	"\t*cpu_s32(sp+0x14) = pri; \\\n"
	"\tlib_osCreateThread(); \\\n"
	"}\n"
	"#define osRecvMesg(mq, msg, flag) \\\n"
	"\t{a0 = mq; a1 = msg; a2 = flag; lib_osRecvMesg();}\n"
	"#define osSpTaskLoad(task) \\\n"
	"\t{a0 = task; lib_osSpTaskLoad();}\n"
	"#define osSpTaskStartGo(task) \\\n"
	"\t{a0 = task; lib_osSpTaskStartGo();}\n"
	"#define osSendMesg(mq, msg, flag) \\\n"
	"\t{a0 = mq; a1 = msg; a2 = flag; lib_osSendMesg();}\n"
	"#define osStartThread(t) \\\n"
	"\t{a0 = t; lib_osStartThread();}\n"
	"#define osWritebackDCacheAll() \\\n"
	"\t{lib_osWritebackDCacheAll();}\n"
	"#define osCreateViManager(pri) \\\n"
	"\t{a0 = pri; lib_osCreateViManager()};\n"
	"#define osCreatePiManager(pri, cmdQ, cmdBuf, cmdMsgCnt) \\\n"
	"{ \\\n"
	"\ta0 = pri; a1 = cmdQ; a2 = cmdBuf; a3 = cmdMsgCnt; \\\n"
	"\tlib_osCreatePiManager(); \\\n"
	"}\n"
	"#define osSetThreadPri(t, pri) \\\n"
	"\t{a0 = t; a1 = pri; lib_osSetThreadPri();}\n"
	"#define osInitialize() \\\n"
	"\t{lib_osInitialize();}\n"
	"\n"
	"#define Na_Load() \\\n"
	"\t(app_8031950C())\n"
	"#define Na_Main() \\\n"
	"\t(app_8031E7B8(), v0)\n"
	"#define Na_Tick() \\\n"
	"\t(app_8031FD84())\n"
	"#define Na_Init() \\\n"
	"\t(app_8032112C())\n"
	"#define Na_BgmPlay(handle, bgm, fadein) \\\n"
	"\t(a0 = handle, a1 = bgm, a2 = fadein, app_803219AC())\n"
	"#define Na_BgmStop(bgm) \\\n"
	"\t(a0 = bgm, app_80321BAC())\n"
	"#define Na_Mode(mode) \\\n"
	"\t(a0 = mode, app_80322230())\n"
	"#define Na_Output(output) \\\n"
	"\t(a0 = output, app_8032231C())\n"
	"\n"
	"extern void lib_80248AF0(void);\n"
	"extern void lib_osPiStartDma_seq(void);\n"
	"\n"
	"extern void app_802469B8(void);\n"
	"extern void app_80246CF0(void);\n"
	"extern void app_80246DF8(void);\n"
	"extern void app_80248AF0(void);\n"
	"extern void app_80249500(void);\n"
)

lib = {
	0x803223B0: "osSetTime",
	0x803223E0: "osMapTLB",
	0x803224A0: "osUnmapTLBAll",
	0x803225A0: "osCreateMesgQueue",
	0x803225D0: "osSetEventMesg",
	0x80322640: "osViSetEvent",
	0x803226B0: "osCreateThread",
	0x80322800: "osRecvMesg",
	0x80322A5C: "osSpTaskLoad",
	0x80322BBC: "osSpTaskStartGo",
	0x80322C00: "osSpTaskYield",
	0x80322C20: "osSendMesg",
	0x80322D70: "osSpTaskYielded",
	0x80322DF0: "osStartThread",
	0x80322F40: "osWritebackDCacheAll",
	0x80322F70: "osCreateViManager",
	0x803232D0: "osViSetMode",
	0x80323340: "osViBlack",
	0x803233B0: "osViSetSpecialFeatures",
	0x80323570: "osCreatePiManager",
	0x803236F0: "osSetThreadPri",
	0x803237D0: "osInitialize",
	0x80323A00: "osViSwapBuffer",
	0x80323A60: "osContStartReadData",
	0x80323B24: "osContGetReadData",
	0x80323CC0: "osContInit",
	0x80324080: "osEepromProbe",
	0x8032411C: "__ull_rem",
	0x80324158: "__ull_div",
	0x80324194: "__ll_lshift",
	0x803241FC: "__ll_div",
	0x80324258: "__ll_mul",
	0x803243B0: "osInvalDCache",
	0x80324460: "osPiStartDma_seq",
	0x80324610: "osInvalICache",
	0x80324690: "osEepromLongRead",
	0x803247D0: "osEepromLongWrite",
	0x80324D74: "guOrtho",
	0x80325010: "guPerspective",
	0x80325070: "osGetTime",
	0x80325138: "__d_to_ull",
	0x803252A4: "__ull_to_d",
	0x80325970: "osAiSetFrequency",
	0x80325D20: "osWritebackDCache",
	0x80325DA0: "osAiGetLength",
	0x80325DB0: "osAiSetNextBuffer",
	0x80327EB0: "osVirtualToPhysical",
}

a00_dst = [
	0x802469B8,
	0x80246CF0,
	0x80246DF8,
	0x80248AF0,
	0x80249500,
]

a00_pat = {
	0x802469B8: [0x03E00008, 0x00000000],
	0x80246CF0: [0x03E00008, 0x00000000],
	0x80246DF8: [0x03E00008, 0x00000000],
	0x80248AF0: [0x03E00008, 0x00000000],
	0x80249500: [0x03E00008, 0x00000000],
}

a00_xpr = {
	0x803197A8: "0x800000",
	0x8031980C: "0x800000",
	0x80319824: "0x800000",
	0x80319844: "0x800000",
	0x8031988C: "0x80000000",
	0x803198A4: "0x80000000",
}

a00_ins = {
	0x802469B8:
	"\tosCreateMesgQueue(0x8033AE08, 0x8033AEC8, 16);\n"
	"\tosCreateMesgQueue(0x8033B010, 0x8033B040, 1);\n"
	"\tosCreateMesgQueue(0x8033B140, 0x8033B158, 1);\n"
	"\tosViSetEvent(0x8033AE08, 0, 1);\n"
	"\tosCreateThread(0x8033AC40, 4, 0x80249500, NULLPTR, 0x80204E00, 20);\n"
	"\tosStartThread(0x8033AC40);\n"
	"\tosCreateThread(0x8033AA90, 5, 0x80248AF0, NULLPTR, 0x80206E00, 10);\n"
	"\tosStartThread(0x8033AA90);\n"
	"\tfor (;;)\n"
	"\t{\n"
	"\t\tosRecvMesg(0x8033AE08, NULLPTR, OS_MESG_BLOCK);\n"
	"\t\tosSendMesg(0x8033B140, 0, OS_MESG_NOBLOCK);\n"
	"\t\tosSendMesg(0x8033B010, 0, OS_MESG_NOBLOCK);\n"
	"\t}\n",
	0x80246CF0:
	"\tosCreateViManager(OS_PRIORITY_VIMGR);\n"
	"\tosCreatePiManager(OS_PRIORITY_PIMGR, 0x8033ADF0, 0x8033AE40, 32);\n"
	"\tosCreateThread(0x8033A8E0, 3, 0x802469B8, NULLPTR, 0x80202E00, 100);\n"
	"\tosStartThread(0x8033A8E0);\n"
	"\tosSetThreadPri(NULLPTR, OS_PRIORITY_IDLE);\n",
	0x80246DF8:
	"\tosInitialize();\n"
	"\tosCreateThread(0x8033A730, 1, 0x80246CF0, NULLPTR, 0x80200E00, 100);\n"
	"\tosStartThread(0x8033A730);\n",
	0x80248AF0: "\tlib_80248AF0();\n",
	0x80249500:
	"\tNa_Load();\n"
	"\tNa_Init();\n"
	"\tfor (;;)\n"
	"\t{\n"
	"\t\tPTR task;\n"
	"\t\tosRecvMesg(0x8033B140, NULLPTR, OS_MESG_BLOCK);\n"
	"\t\tif ((task = Na_Main()) != NULLPTR)\n"
	"\t\t{\n"
	"\t\t\tosWritebackDCacheAll();\n"
	"\t\t\tosSetEventMesg(\n"
	"\t\t\t\tOS_EVENT_SP, *cpu_s32(task+0x40), *cpu_s32(task+0x44)\n"
	"\t\t\t);\n"
	"\t\t\tosSpTaskStart(task);\n"
	"\t\t}\n"
	"\t}\n",
	# audio sleep
	0x80317938: "\tth_yield(TH_BREAK);\n",
}

segment = [
	[0x000CFA30, 0x80314A30, 0x803223B0, a00_dst, a00_pat, a00_xpr, a00_ins],
	[0x000DEA50, 0x80323A50, 0x80323A60, [], {}, {}, {}],
	[0x000E0CD8, 0x80325CD8, 0x80325D20, [], {}, {}, {}],
]

dcall = [
]

cache = [
]
