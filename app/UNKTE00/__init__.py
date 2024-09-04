patch = None

entry = 0x80000400
bss   = [0x800F6910, 0x000A0FC0]
sp    = 0x80152300
main  = 0x80000484
reg = (
	1 <<  2 | # v0
	1 <<  3 | # v1
	1 <<  4 | # a0
	1 <<  5 | # a1
	1 <<  6 | # a2
	1 <<  7 | # a3
	1 <<  8 | # t0
	1 <<  9 | # t1
	1 << 10 | # t2
	1 << 11 | # t3
	1 << 12 | # t4
	1 << 13 | # t5
	1 << 14 | # t6
	1 << 15 | # t7
	1 << 16 | # s0
	1 << 17 | # s1
	1 << 18 | # s2
	1 << 19 | # s3
	1 << 20 | # s4
	1 << 21 | # s5
	1 << 22 | # s6
	1 << 23 | # s7
	1 << 24 | # t8
	1 << 25 | # t9
	1 << 29 | # sp
	1 << 30 | # s8
	1 << 31 | # ra
	1 << 32 | # f0
	1 << 33 | # f2
	1 << 34 | # f4
	1 << 35 | # f6
	1 << 36 | # f8
	1 << 37 | # f10
	1 << 38 | # f12
	1 << 39 | # f14
	1 << 40 | # f16
	1 << 41 | # f18
	1 << 42 | # f20
	1 << 43 | # f22
	1 << 44 | # f24
	1 << 45 | # f26
	1 << 46 | # f28
	1 << 47 | # f30
	1 << 48   # lo/hi
)

header = (
	"#define EEPROM                  1\n"
	"#define AUDIO_FREQ              26800\n"
	"#define GSP_F3D\n"
	"#define GSP_F3DEX\n"
	"#define ASP_MAIN\n"
	"#define ASP_MAIN1\n"
	"\n"
	"#define __osExceptionPreamble   0x800D11B0\n"
)

lib = {
	0x800CBF70: "osCreateThread",
	0x800CC0C0: "osInitialize",
	0x800CC360: "osStartThread",
	0x800CC4B0: "osCreateViManager",
	0x800CC850: "osViSetMode",
	0x800CC8C0: "osViBlack",
	0x800CC930: "osViSetSpecialFeatures",
	0x800CCAF0: "osCreatePiManager",
	0x800CCC80: "osSetThreadPri",
	0x800CCD60: "osCreateMesgQueue",
	0x800CCD90: "osViSetEvent",
	0x800CCE00: "osSetEventMesg",
	0x800CCF8C: "osSpTaskLoad",
	0x800CD0EC: "osSpTaskStartGo",
	0x800CD130: "osContInit",
	0x800CD4F0: "osContStartReadData",
	0x800CD5B4: "osContGetReadData",
	0x800CD750: "osRecvMesg",
	0x800CD890: "osWritebackDCacheAll",
	0x800CD8C0: "osSendMesg",
	0x800CDA10: "osViSwapBuffer",
	0x800CDB00: "osInvalICache",
	0x800CDB80: "osInvalDCache",
	0x800CDC30: "osPiStartDma",
	0x800CDD40: "osSpTaskYield",
	0x800CDD60: "osSpTaskYielded",
	0x800CDDE0: "osGetTime",
	0x800CDE9C: "__ull_rem",
	0x800CDED8: "__ull_div",
	0x800CDF7C: "__ll_div",
	0x800CDFD8: "__ll_mul",
	0x800CE130: "__osGetCurrFaultedThread",
	0x800CE2A4: "guOrtho",
	0x800CE310: "osSetTime",
	0x800CE340: "osEepromProbe",
	0x800CE3B0: "osPfsIsPlug",
	0x800CE720: "osPfsInitPak",
	0x800CE8E0: "osPfsNumFiles",
	0x800CEA30: "osPfsFileState",
	0x800CED20: "osPfsFreeBlocks",
	0x800CF330: "guPerspective",
	0x800CF880: "osPfsFindFile",
	0x800CFA40: "osPfsDeleteFile",
	0x800D0050: "osEepromLongWrite",
	0x800D0190: "osEepromLongRead",
	0x800D03CC: "osPfsReadWriteFile",
	0x800D07D0: "osPfsAllocateFile",
	0x800D0F80: "osAiSetFrequency",
	0x800D10E0: "osAiGetLength",
	0x800D10F0: "osAiSetNextBuffer",
	0x800D11A0: "osGetCount",
}

a00_pat = {
	0x80040C58: [0x00000000],
	0x80040C8C: [0x03E00008],
}

segment = [
	[0x00001050, 0x80000450, 0x800CBF70, [], a00_pat, {}, {}],
	[0x000CE660, 0x800CDA60, 0x800CDB00, [], {}, {}, {}],
	[0x000CED40, 0x800CE140, 0x800CE150, [], {}, {}, {}],
	[0x000CFA70, 0x800CEE70, 0x800CF100, [], {}, {}, {}],
	[0x000CFF90, 0x800CF390, 0x800CF880, [], {}, {}, {}],
	[0x000D4120, 0x800D3520, 0x800D3830, [], {}, {}, {}],
	[0x000D49A0, 0x800D3DA0, 0x800D4010, [], {}, {}, {}],
	[0x000D6C60, 0x800D6060, 0x800D72F0, [], {}, {}, {}],
	[0x000D8BE0, 0x800D7FE0, 0x800D8F70, [], {}, {}, {}],
	# a01
	[0x000F7510, 0x8028DF00, 0x802B8790, [], {}, {}, {}],
	# a02
	[0x00123640, 0x80280000, 0x80284E40, [], {}, {}, {}],
]

dcall = [
]

cache = [
	[0x00966260, 0x00BD97A0], # audio
]
