def patch(data):
    return data

entry = 0x80000400
bss   = [0x800F6910, 0x000A0FC0]
sp    = 0x80152300
main  = 0x80000484
reg   = (
    0x0000000000000004 | # v0
    0x0000000000000008 | # v1
    0x0000000000000010 | # a0
    0x0000000000000020 | # a1
    0x0000000000000040 | # a2
    0x0000000000000080 | # a3
    0x0000000000000100 | # t0
    0x0000000000000200 | # t1
    0x0000000000000400 | # t2
    0x0000000000000800 | # t3
    0x0000000000001000 | # t4
    0x0000000000002000 | # t5
    0x0000000000004000 | # t6
    0x0000000000008000 | # t7
    0x0000000000010000 | # s0
    0x0000000000020000 | # s1
    0x0000000000040000 | # s2
    0x0000000000080000 | # s3
    0x0000000000100000 | # s4
    0x0000000000200000 | # s5
    0x0000000000400000 | # s6
    0x0000000000800000 | # s7
    0x0000000001000000 | # t8
    0x0000000002000000 | # t9
    0x0000000020000000 | # sp
    0x0000000040000000 | # s8
    0x0000000080000000 | # ra
    0x0000000100000000 | # f0
    0x0000000200000000 | # f2
    0x0000000400000000 | # f4
    0x0000000800000000 | # f6
    0x0000001000000000 | # f8
    0x0000002000000000 | # f10
    0x0000004000000000 | # f12
    0x0000008000000000 | # f14
    0x0000010000000000 | # f16
    0x0000020000000000 | # f18
    0x0000040000000000 | # f20
    0x0000080000000000 | # f22
    0x0000100000000000 | # f24
    0x0000200000000000 | # f26
    0x0000400000000000 | # f28
    0x0000800000000000 | # f30
    0x0001000000000000   # lo/hi
)

header = (
    # "#define LIB_DYNRES\n"
    "#define GSP_F3D\n"
    "#define GSP_F3DEX\n"
    # "#define GSP_FOG\n"
    "#define ASP_MAIN\n"
    "#define ASP_MAIN1\n"
    # "#define ASP_MAIN2\n"
    "#ifdef GEKKO\n"
    "#define APP_BORDER 8\n"
    "#else\n"
    "#define APP_BORDER 0\n"
    "#endif\n"
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
    0x800CDA60: "bzero",
    0x800CDB00: "osInvalICache",
    0x800CDB80: "osInvalDCache",
    0x800CDC30: "osPiStartDma",
    0x800CDD40: "osSpTaskYield",
    0x800CDD60: "osSpTaskYielded",
    0x800CDDE0: "osGetTime",
    0x800CDE9C: "__ull_rem", #
    0x800CDED8: "__ull_div",
    0x800CDF7C: "__ll_div", #
    0x800CDFD8: "__ll_mul",
    0x800CE130: "__osGetCurrFaultedThread", #
    0x800CE140: "sqrtf",
    0x800CE2A4: "guOrtho",
    0x800CE310: "osSetTime",
    0x800CE340: "osEepromProbe",
    0x800CE3B0: "osPfsIsPlug", #
    0x800CE720: "osPfsInitPak", #
    0x800CE8E0: "osPfsNumFiles", #
    0x800CEA30: "osPfsFileState", #
    0x800CED20: "osPfsFreeBlocks", #
    0x800CF004: "guRotate",
    0x800CF0B4: "guScale",
    0x800CF330: "guPerspective",
    0x800CF390: "guLookAtF", #
    0x800CF648: "guLookAt", #
    0x800CF708: "guTranslate",
    0x800CF774: "osSyncPrintf", #
    0x800CF820: "guMtxCatL", #
    0x800CF880: "osPfsFindFile", #
    0x800CFA40: "osPfsDeleteFile", #
    0x800D0050: "osEepromLongWrite",
    0x800D0190: "osEepromLongRead",
    0x800D03CC: "osPfsReadWriteFile", #
    0x800D07D0: "osPfsAllocateFile", #
    0x800D0F80: "osAiSetFrequency",
    0x800D10E0: "osAiGetLength",
    0x800D10F0: "osAiSetNextBuffer",
    0x800D11A0: "osGetCount",
    0x800D1C00: "osWritebackDCache",
    0x800D3520: "bcopy",
    0x800D3830: "osVirtualToPhysical",
    0x800D39C0: "osSetTimer", #
    0x800D60F0: "sinf",
    0x800D62B0: "cosf",
    0x800D7180: "guMtxCatF",
}

a00_pat = {
    0x80040C58: [0x00000000],
    0x80040C8C: [0x03E00008],
}

segment = [
    # 0x800D8F70
    [0x00001050, 0x80000450, 0x800CBF70, [], a00_pat, {}, {}],
    [0x000F7510, 0x8028DF00, 0x802B8790, [], {}, {}, {}],
    [0x00123640, 0x80280000, 0x80284E40, [], {}, {}, {}],
]

dcall = [
]

cache = [
    [0x00966260, 0x00BD97A0], # audio
]
