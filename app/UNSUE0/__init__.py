
def patch(data):
    return data

entry = 0x80000400
bss   = [0x800AF860, 0x00010840]
sp    = 0x803FFFF0
main  = 0x80000E64
reg   = (
    0x0000000000000004 | # v0
    0x0000000000000008 | # v1
    0x0000000000000010 | # a0
    0x0000000000000020 | # a1
    0x0000000000000040 | # a2
    0x0000000000000080 | # a3
    0x0000000020000000 | # sp
    0x0000000100000000 | # f0
    0x0000004000000000 | # f12
    0x0000008000000000   # f14
)

header = (
    "#define GSP_F3DEX2\n"
    "#define ASP_MAIN2\n"
    "\n"
    "#define __osExceptionPreamble   0x803FFFF0\n"
)

lib = {
    0x800040B0: "bzero",
    0x80004150: "osPiGetStatus",
    0x80004160: "__osSpSetPc",
    0x80004190: "osStartThread",
    0x80004310: "osCreateThread",
    0x800043E0: "osSetThreadPri",
    0x800044B0: "osWritebackDCache",
    0x80004530: "osCreatePiManager",
    0x800046C0: "osCreateMesgQueue",
    0x8000516C: "8000516C",
    0x800051C0: "800051C0",
    0x8000546C: "8000546C",
    0x80005494: "80005494",
    0x800054C8: "800054C8",
    0x800057AC: "800057AC",
    0x800058D0: "800058D0",
    0x80005A08: "80005A08",
    0x80005AB4: "80005AB4",
    0x80005B08: "80005B08",
    0x80005B64: "80005B64",
    0x80005BC8: "80005BC8",
    0x80005D2C: "80005D2C",
    0x80005EEC: "80005EEC",
    0x80005F74: "80005F74",
    0x80005F90: "80005F90",
    0x80007D00: "osPiRawStartDma",
    0x80007DD0: "osInvalDCache",
    0x80007E80: "osInitialize",
    0x80008090: "osSendMesg",
    0x800081C0: "osRecvMesg",
    0x800082F0: "osGetTime",
    0x80008418: "guMtxL2F",
    0x80008510: "guMtxIdent",
    0x800087B0: "guPerspective",
    0x80008980: "osEepromProbe",
    0x80008A00: "osEepromWrite",
    0x80008DE0: "osEepromLongRead",
    0x80008E50: "80008E50",
    0x80008F10: "osEepromRead",
    0x80009160: "osEepromLongWrite",
    0x80009250: "80009250",
    0x80009290: "guMtxCatL",
    0x800093E4: "guTranslate",
    0x800094C0: "guScale",
    0x80009540: "osViSetSpecialFeatures",
    0x800096B0: "osContInit",
    0x800099C0: "osContStartReadData",
    0x80009A48: "osContGetReadData",
    0x80009BA0: "osSetEventMesg",
    0x80009C00: "guRandom",
    0x80009C30: "osPiStartDma",
    0x80009CE0: "80009CE0",
    0x80009D00: "osMotorStop",
    0x80009EC0: "osMotorStart",
    0x8000A080: "osMotorInit",
    0x8000A550: "8000A550",
    0x8000A740: "osWritebackDCacheAll",
    0x8000A770: "sinf",
    0x8000A910: "osCreateViManager",
    0x8000AC40: "osViSetMode",
    0x8000AC90: "osViBlack",
    0x8000ACF0: "osViSetEvent",
    0x8000AD50: "osSetIntMask",
    0x8000ADF0: "__assert",
    0x8000AE00: "osSpTaskYielded",
    0x8000AE50: "osViGetCurrentFramebuffer",
    0x8000AE90: "osViGetNextFramebuffer",
    0x8000AED0: "osViSwapBuffer",
    0x8000AF20: "osSpTaskLoad",
    0x8000B12C: "osSpTaskStartGo",
    0x8000B160: "osDpSetNextBuffer",
    0x8000B200: "osSpTaskYield",
}

segment = [
    [0x0000103C, 0x8000043C, 0x80000500, [], {}, {}, {}], # main
    [0x000019D0, 0x80000DD0, 0x80001490, [], {}, {}, {}], # app

    [0x00004150, 0x80003550, 0x800040B0, [], {}, {}, {}], # sched
    # [0x00004150, 0x80003550, 0x80017DE0, [], {}, {}, {}], # lib

    [0x0001EA00, 0x8001DE00, 0x8009F070, [], {}, {}, {}], # game
]

dcall = [
]

cache = [
]
