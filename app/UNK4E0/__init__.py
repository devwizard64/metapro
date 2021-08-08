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
    0x8002D6E0: "sinf",
    0x8002D9BC: "osSpTaskLoad",
    0x8002DB4C: "osSpTaskStartGo",
    0x8002DB90: "osDestroyThread",
    0x8002DD08: "__ull_div",
    0x8002DE08: "__ll_mul",
    0x8002E1A8: "8002E1A8",
    0x8002E1B4: "8002E1B4",
    0x8002E1C0: "8002E1C0",
    0x8002E1E0: "8002E1E0",
    0x8002E234: "8002E234",
    0x8002E374: "8002E374",
    0x8002E3D0: "8002E3D0",
    0x8002E3F0: "8002E3F0",
    0x8002E428: "8002E428",
    0x8002E8B0: "bzero",
    0x8002F040: "__osMotorAccess",
    0x8002F2B4: "osMotorInit",
    0x8002F4D0: "osContReset",
    0x8002F810: "osEepromWrite",
    0x8002FBE0: "osCreateThread",
    0x8002FD30: "osContStartReadData",
    0x8002FDB4: "osContGetReadData",
    0x8002FF10: "osEepromLongRead",
    0x8002FFA0: "osVirtualToPhysical",
    0x80030020: "osWritebackDCache",
    0x800300A0: "osInitialize",
    0x800301EC: "800301EC",
    0x80030410: "osViGetNextFramebuffer",
    0x80030590: "osEPiLinkHandle",
    0x800305E0: "osViBlack",
    0x800306A0: "osSpTaskYield",
    0x80030794: "80030794",
    0x80030C10: "guMtxIdentF",
    0x80030D80: "osViSetMode",
    0x80030DF0: "osPfsAllocateFile",
    0x800314D0: "osGetCount",
    0x80032060: "osEepromProbe",
    0x800320D0: "osPfsFindFile",
    0x80032280: "osCreatePiManager",
    0x80032834: "80032834",
    0x80032870: "80032870",
    0x800328AC: "800328AC",
    0x800328F4: "800328F4",
    0x8003293C: "8003293C",
    0x8003295C: "8003295C",
    0x80032AB0: "osSetEventMesg",
    0x80032B20: "sqrtf",
    0x80032B30: "osAfterPreNMI",
    0x80032B50: "osContStartQuery",
    0x80032BD4: "osContGetQuery",
    0x80032C70: "80032C70",
    0x80032CA8: "80032CA8",
    0x80032E18: "80032E18",
    0x80032F68: "80032F68",
    0x800330CC: "800330CC",
    0x8003319C: "8003319C",
    0x800331E4: "800331E4",
    0x80033248: "80033248",
    0x80033270: "_Printf",
    0x800338C0: "osEPiStartDma",
    0x80033A40: "memcpy",
    0x80033AE0: "osCreateMesgQueue",
    0x80033B10: "osInvalICache",
    0x80033B90: "osInvalDCache",
    0x80033C40: "osEepromLongWrite",
    0x80033EE0: "osSetThreadPri",
    0x80033FC0: "osGetThreadPri",
    0x80033FE0: "osViSwapBuffer",
    0x80034030: "guMtxXFMF",
    0x800340D0: "guMtxCatF",
    0x80034470: "osSpTaskYielded",
    0x80034610: "osGetTime",
    0x80034AF0: "osAiSetFrequency",
    0x80034C50: "guNormalize",
    0x80034CF0: "__osGetActiveQueue",
    0x80035010: "alCopy",
    0x800350F0: "osPfsDeleteFile",
    0x80035810: "cosf",
    0x80035980: "osSetTime",
    0x800359B0: "osViSetEvent",
    0x80035A20: "osCartRomInit",
    0x80035B20: "guS2DInitBg",
    0x80035D30: "80035D30",
    0x80037080: "alCents2Ratio",
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
    0x8002013C:
        "    t8 = v0;\n"
        "    lib_osAiGetLength();\n"
        "    t9 = v0;\n"
        "    v0 = t8;\n",
    0x80020804:
        "    t9 = v0;\n"
        "    lib_osAiGetLength();\n"
        "    t6 = v0;\n"
        "    v0 = t9;\n",
}

a01_ins = {
    # 800A74B0
    0x800A74B8: "    thread_yield(THREAD_YIELD_BREAK);\n",
    # 800A74D8
    0x800A7508: "    thread_yield(THREAD_YIELD_BREAK);\n",
}

segment = [
    # 0x80039230
    [0x00001050, 0x80000450, 0x8002D2F0, [], a00_pat, {}, a00_ins],
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
