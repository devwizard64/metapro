import struct

def xor(data):
	return B"".join([
		struct.pack(">I", struct.unpack(">I", data[i:i+4])[0] ^ 0xFDB9ECA8)
		for i in range(0, len(data), 4)
	])

def patch(data):
	offs  = 0x80300000-0x002501C0
	start = 0x803000E0-offs
	end   = 0x80300160-offs
	return data[:start] + xor(data[start:start+0x10]) + struct.pack(
		">I", 0x0C000000 | (0x80002E48 >> 2 & 0x03FFFFFF)
	) + xor(data[start+0x14:end]) + data[end:]

entry = 0x80000400
bss   = [0x80042B90, 0x000589B0]
sp    = 0x80042D90
main  = 0x80000870
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
	"#define EEPROM                  2\n"
	"#define AUDIO_FREQ              32000\n"
	"#define GSP_F3DEX\n"
	"#define GSP_F3DEX2\n"
	"#define GSP_FOG\n"
	"#define ASP_NAUDIO\n"
	"\n"
	"#define __osExceptionPreamble   0x8002DF60\n"
)

lib = {
	0x8002D2F0: "osSendMesg",
	0x8002D440: "osStopThread",
	0x8002D500: "osRecvMesg",
	0x8002D640: "osSetIntMask",
	0x8002D9BC: "osSpTaskLoad",
	0x8002DB4C: "osSpTaskStartGo",
	0x8002DB90: "osDestroyThread",
	0x8002DCCC: "__ull_rem",
	0x8002DD08: "__ull_div",
	0x8002DDAC: "__ll_div",
	0x8002DE08: "__ll_mul",
	0x8002F040: "__osMotorAccess",
	0x8002F2B4: "osMotorInit",
	0x8002F4D0: "osContInit",
	0x8002F810: "osEepromWrite",
	0x8002FBE0: "osCreateThread",
	0x8002FD30: "osContStartReadData",
	0x8002FDB4: "osContGetReadData",
	0x8002FF10: "osEepromLongRead",
	0x8002FFA0: "osVirtualToPhysical",
	0x80030020: "osWritebackDCache",
	0x800300A0: "osInitialize",
	0x80030410: "osViGetNextFramebuffer",
	0x80030590: "osEPiLinkHandle",
	0x800305E0: "osViBlack",
	0x800306A0: "osSpTaskYield",
	0x80030794: "osPfsReadWriteFile",
	0x80030D80: "osViSetMode",
	0x80030DF0: "osPfsAllocateFile",
	0x800314D0: "osGetCount",
	0x80032060: "osEepromProbe",
	0x800320D0: "osPfsFindFile",
	0x80032280: "osCreatePiManager",
	0x80032AB0: "osSetEventMesg",
	0x80032B30: "osAfterPreNMI",
	0x80032B50: "osContStartQuery",
	0x80032BD4: "osContGetQuery",
	0x800338C0: "osEPiStartDma",
	0x80033AE0: "osCreateMesgQueue",
	0x80033B10: "osInvalICache",
	0x80033B90: "osInvalDCache",
	0x80033C40: "osEepromLongWrite",
	0x80033EE0: "osSetThreadPri",
	0x80033FC0: "osGetThreadPri",
	0x80033FE0: "osViSwapBuffer",
	0x80034470: "osSpTaskYielded",
	0x80034610: "osGetTime",
	0x80034AF0: "osAiSetFrequency",
	0x80034CF0: "__osGetActiveQueue",
	0x800350F0: "osPfsDeleteFile",
	0x80035980: "osSetTime",
	0x800359B0: "osViSetEvent",
	0x80035A20: "osCartRomInit",
	0x80035D30: "osPfsInitPak",
	0x80038880: "osDpSetNextBuffer",
	0x80038980: "osCreateViManager",
	0x80038CA0: "osWritebackDCacheAll",
	0x80038CD0: "osStartThread",
	0x80038E20: "osViSetYScale",
	0x80038E80: "osAiSetNextBuffer",
	0x80038F60: "osEepromRead",
	0x800391B0: "osViGetCurrentFramebuffer",
}

# a00
a00_pat = {
	# imem check
	0x80000480: [0x24180001, 0x00000000, 0x00000000, 0x00000000, 0x00000000],
	0x800004A0: [0x00000000, 0x00000000, 0x00000000],
	# dmem check
	0x800004B0: [0x24180001, 0x00000000, 0x00000000, 0x00000000, 0x00000000],
	0x800004D0: [0x00000000, 0x00000000, 0x00000000],
	# dma_read
	0x80002C70: [0x1000001F],
	# 8001FD64
	0x8002013C: [0x00000000, 0x00000000],
	0x80020804: [0x00000000, 0x00000000],
}

a00_ins = {
	# 8001FD64
	0x8002013C: "\tt9 = audio_size();\n",
	0x80020804: "\tt6 = audio_size();\n",
}

a01_ins = {
	# 800A74B0
	0x800A74B8: "\tth_yield(TH_BREAK);\n",
	# 800A74D8
	0x800A7508: "\tth_yield(TH_BREAK);\n",
}

segment = [
	[0x00001050, 0x80000450, 0x8002D2F0, [], a00_pat, {}, a00_ins],
	[0x0002E2E0, 0x8002D6E0, 0x8002D8A0, [], {}, {}, {}],
	[0x0002F4B0, 0x8002E8B0, 0x8002E950, [], {}, {}, {}],
	[0x00031810, 0x80030C10, 0x80030C98, [], {}, {}, {}],
	[0x00033720, 0x80032B20, 0x80032B30, [], {}, {}, {}],
	[0x00033800, 0x80032C00, 0x800338C0, [], {}, {}, {}],
	[0x00034640, 0x80033A40, 0x80033AE0, [], {}, {}, {}],
	[0x00034C30, 0x80034030, 0x80034240, [], {}, {}, {}],
	[0x00035850, 0x80034C50, 0x80034CE0, [], {}, {}, {}],
	[0x00035C10, 0x80035010, 0x80035090, [], {}, {}, {}],
	[0x00036410, 0x80035810, 0x80035980, [], {}, {}, {}],
	[0x00036720, 0x80035B20, 0x80035CA0, [], {}, {}, {}],
	[0x00037C80, 0x80037080, 0x800370D0, [], {}, {}, {}],
	[0x00038530, 0x80037930, 0x80038830, [], {}, {}, {}],
	[0x00043790, 0x8009B540, 0x800BE3E0, [], {}, {}, a01_ins],
	[0x0007EC10, 0x800F61A0, 0x80123E90, [], {}, {}, {}],
	[0x000B1B40, 0x80151100, 0x80190310, [], {}, {}, {}],
	[0x000F8630, 0x80151100, 0x80159FC0, [], {}, {}, {}],
	[0x00103BB0, 0x8015C740, 0x80185FA0, [], {}, {}, {}],
	[0x00135490, 0x80151100, 0x80154DC0, [], {}, {}, {}],
	[0x0013E8F0, 0x80198880, 0x801C27D0, [], {}, {}, {}],
	[0x00174740, 0x801D0C60, 0x801D7880, [], {}, {}, {}],
	[0x0017ECB0, 0x801D0C60, 0x8021BAB0, [], {}, {}, {}],
	[0x001CBF50, 0x801DB1E0, 0x801F33F0, [], {}, {}, {}],
	[0x001E5AA0, 0x801DB1E0, 0x801E0B00, [], {}, {}, {}],
	[0x001EB520, 0x801DB1E0, 0x801E2AA0, [], {}, {}, {}],
	[0x001F3160, 0x801DB1E0, 0x801E5A80, [], {}, {}, {}],
	[0x001FDDD0, 0x801DB1E0, 0x801E2EA0, [], {}, {}, {}],
	[0x00205D40, 0x801DB1E0, 0x801E6450, [], {}, {}, {}],
	[0x00211490, 0x801DB1E0, 0x801EF4C0, [], {}, {}, {}],
	[0x002263D0, 0x801DB1E0, 0x801E4E60, [], {}, {}, {}],
	[0x002308C0, 0x8021DF20, 0x802273E0, [], {}, {}, {}],
	[0x0023E630, 0x8021DF20, 0x8022F040, [], {}, {}, {}],
	[0x002501C0, 0x80300000, 0x803000E0, [], {}, {}, {}],
	[0x002502A0, 0x80300170, 0x803001F0, [], {}, {}, {}],
]

dcall = [
	0x00151100,
	0x0015C740,
	0x001D0C60,
	0x001DB1E0,
	0x0021DF20,
]

cache = [
	[0x00000000, 0x02000000], # tmp
]
