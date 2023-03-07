patch = None

entry = 0x80246000
bss   = [0x80339210, 0x0002CEE0]
sp    = 0x80200A00
main  = 0x80246DC4
reg   = (
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
    "#define EEPROM                  1\n"
    "#define VIDEO_DYNRES\n"
    "#define AUDIO_FREQ              32000\n"
    "#define GSP_F3D\n"
    "#define GSP_F3D_20D\n"
    "#define ASP_MAIN\n"
    "#define ASP_MAIN1\n"
    "#ifdef GEKKO\n"
    "#define BORDER_HT  8\n"
    "#else\n"
    "#define BORDER_HT  0\n"
    "#endif\n"
    "\n"
    "#define __osExceptionPreamble   0x803266D0\n"
)

lib = {
    0x80321480: "osSetTime",
    0x803214B0: "osMapTLB",
    0x80321570: "osUnmapTLBAll",
    0x80321670: "osCreateMesgQueue",
    0x803216A0: "osSetEventMesg",
    0x80321710: "osViSetEvent",
    0x80321780: "osCreateThread",
    0x803218D0: "osRecvMesg",
    0x80321B2C: "osSpTaskLoad",
    0x80321C8C: "osSpTaskStartGo",
    0x80321CD0: "osSpTaskYield",
    0x80321CF0: "osSendMesg",
    0x80321E40: "osSpTaskYielded",
    0x80321EC0: "osStartThread",
    0x80322010: "osWritebackDCacheAll",
    0x80322040: "osCreateViManager",
    0x803223A0: "osViSetMode",
    0x80322410: "osViBlack",
    0x80322480: "osViSetSpecialFeatures",
    0x80322640: "osCreatePiManager",
    0x803227C0: "osSetThreadPri",
    0x803228A0: "osInitialize",
    0x80322AD0: "osViSwapBuffer",
    0x80322B30: "osContStartReadData",
    0x80322BF4: "osContGetReadData",
    0x80322D90: "osContInit",
    0x80323150: "osEepromProbe",
    0x803231EC: "__ull_rem",
    0x80323228: "__ull_div",
    0x80323264: "__ll_lshift",
    0x803232CC: "__ll_div",
    0x80323328: "__ll_mul",
    0x80323480: "osInvalDCache",
    0x80323530: "osPiStartDma",
    0x803236E0: "osInvalICache",
    0x80323760: "osEepromLongRead",
    0x803238A0: "osEepromLongWrite",
    0x80323E44: "guOrtho",
    0x803240E0: "guPerspective",
    0x80324140: "osGetTime",
    0x80324208: "__d_to_ull",
    0x80324374: "__ull_to_d",
    0x80324A40: "osAiSetFrequency",
    0x80324DF0: "osWritebackDCache",
    0x80324E70: "osAiGetLength",
    0x80324E80: "osAiSetNextBuffer",
    0x80326F30: "osVirtualToPhysical",
}

a00_pat = {
    # video_init_cimg
    0x80247460: [0x35CE0000],
    0x80247470: [0x371803C0],
    # video_clear
    0x80247570: [0x3508C3BC],
    0x8024757C: [0x240A0000],
    # video_draw_border
    0x80247918: [0x1000001A, 0x00000000],
    # scene_draw
    0x8027AEBC: [0x37180000],
    0x8027AECC: [0x350803C0],
    0x8027AF48: [0x35080000],
    0x8027AF58: [0x354A03C0],
    0x8027AFD4: [0x37390000],
    0x8027AFE4: [0x352903C0],
    # draw_background
    0x8027C3EC: [0x3508C3BC],
    0x8027C3F8: [0x24180000],
    # object spawn
    0x802C9164: [0x3C018036, 0x2421FB18, 0x1000000D, 0xAFA1001C],
    # dprint_clamp
    0x802D60A0: [0x00000000],
}

a00_xpr = {
    0x80256D44: "(int)video_l + 28", # staff str l
    0x80256D7C: "(int)video_r - 28", # staff str r
    0x8027A354: "(int)video_l + 30 + 6*5", # "press" x
    0x8027A35C: "BORDER_HT + 12 + 18", # "press" y
    0x8027A368: "(int)video_l + 30 + 6*5", # "start" x
    0x8027A370: "BORDER_HT + 12", # "start" y
    0x8027AD24: "(int)(video_r-video_l)", # wipe fadeout radius
    0x8027AD88: "(int)(video_r-video_l)", # wipe fadein radius
    0x802CAE20: "(int)video_l", # wipe v0 x
    0x802CAE64: "(int)video_r", # wipe v1 x
    0x802CAEA8: "(int)video_r", # wipe v2 x
    0x802CAEEC: "(int)video_l", # wipe v3 x
    0x802CC300: "0x0080 + 0x0028", # reticle gfx
    0x802CC32C: "(int)(-3.6F * (video_r-video_l))", # reticle v0 s
    0x802CC350: "(int)video_l", # reticle v0 x
    0x802CC36C: "(int)( 3.6F * (video_r-video_l))", # reticle v1 s
    0x802CC390: "(int)video_r", # reticle v1 x
    0x802CC3AC: "(int)( 3.6F * (video_r-video_l))", # reticle v2 s
    0x802CC3D0: "(int)video_r", # reticle v2 x
    0x802CC3EC: "(int)(-3.6F * (video_r-video_l))", # reticle v3 s
    0x802CC410: "(int)video_l", # reticle v3 x
    0x802DA894: "(int)video_r - 30", # pause red coin
    0x802E2844: "(int)video_l + 22 + 16*0", # life "," x
    0x802E284C: "240 - BORDER_HT - 6 - 16", # life "," y
    0x802E2858: "(int)video_l + 22 + 16*1", # life "*" x
    0x802E2860: "240 - BORDER_HT - 6 - 16", # life "*" y
    0x802E2874: "(int)video_l + 22 + 16*2", # life "%d" x
    0x802E287C: "240 - BORDER_HT - 6 - 16", # life "%d" y
    # 0x802E28A8: "(int)(320)/2 + 8 + 16*0", # coin "+" x
    0x802E28B0: "240 - BORDER_HT - 6 - 16", # coin "+" y
    # 0x802E28BC: "(int)(320)/2 + 8 + 16*1", # coin "*" x
    0x802E28C4: "240 - BORDER_HT - 6 - 16", # coin "*" y
    # 0x802E28D8: "(int)(320)/2 + 8 + 16*2", # coin "%d" x
    0x802E28E0: "240 - BORDER_HT - 6 - 16", # coin "%d" y
    0x802E295C: "(int)video_r - (22 + 16+16+12*2) + 16*0", # star "-" x
    0x802E2964: "240 - BORDER_HT - 6 - 16", # star "-" y
    0x802E2980: "(int)video_r - (22 + 16+16+12*2) + 16*1", # star "*" x
    0x802E2988: "240 - BORDER_HT - 6 - 16", # star "*" y
    0x802E29A8: "a0 + (int)video_r - (22 + 16+16+12*2) + 16*1", # star "%d" x
    0x802E29B8: "240 - BORDER_HT - 6 - 16", # star "%d" y
    0x802E2A04: "a0 + (int)video_r - (22 + 78)", # key "/" x
    # 0x802E2A0C: "", # key "/" y
    0x802E2B10: # time "time" x
        "(int)video_r - (22 + 12*1+3 + 9 + 12*2+1 + 10 + 10 + 12*5-1)",
    0x802E2B18: "240 - BORDER_HT - 31 - 16", # time "time" y
    0x802E2B24: # time "%0d" x
        "(int)video_r - (22 + 12*1+3 + 9 + 12*2+1 + 10 + 10)",
    0x802E2B28: "240 - BORDER_HT - 31 - 16", # time "%0d" y
    0x802E2B3C: "(int)video_r - (22 + 12*1+3 + 9 + 12*2+1)", # time "%02d" x
    0x802E2B40: "240 - BORDER_HT - 31 - 16", # time "%02d" y
    0x802E2B54: "(int)video_r - (22 + 12*1+3)", # time "%d" x
    0x802E2B58: "240 - BORDER_HT - 31 - 16", # time "%d" y
    0x802E2B9C: "(int)video_r - (22 + 12*1+3 + 9 + 12*2+1 + 10)", # time ' x
    0x802E2BA0: "BORDER_HT + 24", # time ' y
    0x802E2BB0: "(int)video_r - (22 + 12*1+3 + 9)", # time " x
    0x802E2BB4: "BORDER_HT + 24", # time " y
    0x802E2C48: "(int)video_r - (22 + 16+16)", # camera x
    0x802E2C50: "240 - BORDER_HT - 7 - 20", # camera y
}

a00_ins = {
    # draw_object_isvisible
    0x8027D018: "    f18.f[IX] *= video_aspect;\n",
    # reticle border
    0x802CC6BC:
        "{\n"
        "    int x = 0.15F * (video_r-video_l);\n"
        "    at = *cpu_s32(sp+0x7C);\n"
        "    *cpu_s32(sp+0x7C) = at+0x28;\n"
        "    *cpu_s32(at+0x00) = 0xBA001402;\n"
        "    *cpu_s32(at+0x04) = 0x00300000;\n"
        "    *cpu_s32(at+0x08) = 0xF7000000;\n"
        "    *cpu_s32(at+0x0C) = 0x00010001;\n"
        "    *cpu_s32(at+0x10) = 0xF60003BC | ((  x-1) & 0x3FF) << 14;\n"
        "    *cpu_s32(at+0x14) = 0x00000000;\n"
        "    *cpu_s32(at+0x18) = 0xF64FC3BC;\n"
        "    *cpu_s32(at+0x1C) = 0x00000000 | ((320-x) & 0x3FF) << 14;\n"
        "    *cpu_s32(at+0x20) = 0xBA001402;\n"
        "    *cpu_s32(at+0x24) = 0x00000000;\n"
        "}\n",
    # pause
    0x802DA554: "    ARG_F(a1) = video_l;\n",
    0x802DA570: "    ARG_F(a1) = (1.0F/128) * (video_r-video_l);\n",
    # hud draw power
    0x802E2344: "    t8 -= BORDER_HT-8;\n",
    # audio sleep
    0x80316918: "    th_yield(TH_BREAK);\n",
}

segment = [
    [0x00001050, 0x80246050, 0x8027EFE0, [], a00_pat, a00_xpr, a00_ins],
    [0x00039FE0, 0x8027EFE0, 0x8029BFF0, [], a00_pat, a00_xpr, a00_ins],
    [0x00056FF0, 0x8029BFF0, 0x802CAAE0, [], a00_pat, a00_xpr, a00_ins],
    [0x00085AE0, 0x802CAAE0, 0x802D5320, [], a00_pat, a00_xpr, a00_ins],
    [0x00090320, 0x802D5320, 0x802F8680, [], a00_pat, a00_xpr, a00_ins],
    [0x000B3680, 0x802F8680, 0x80313920, [], a00_pat, a00_xpr, a00_ins],
    [0x000CE920, 0x80313920, 0x80321480, [], a00_pat, a00_xpr, a00_ins],
    [0x000DC5C0, 0x803215C0, 0x80321670, [], a00_pat, a00_xpr, a00_ins],
    [0x000DDB20, 0x80322B20, 0x80322B30, [], a00_pat, a00_xpr, a00_ins],
    [0x000DE640, 0x80323640, 0x803236E0, [], a00_pat, a00_xpr, a00_ins],
    [0x000DE9E0, 0x803239E0, 0x80323CF0, [], a00_pat, a00_xpr, a00_ins],
    [0x000DF3E0, 0x803243E0, 0x80324A40, [], a00_pat, a00_xpr, a00_ins],
    [0x000DFDA8, 0x80324DA8, 0x80324DF0, [], a00_pat, a00_xpr, a00_ins],
    [0x000E0330, 0x80325330, 0x80326560, [], a00_pat, a00_xpr, a00_ins],
    [0x000E3510, 0x80328510, 0x80328810, [], a00_pat, a00_xpr, a00_ins],
    [0x000E3850, 0x80328850, 0x80329920, [], a00_pat, a00_xpr, a00_ins],
    [0x000E5120, 0x8032A120, 0x8032A2B0, [], a00_pat, a00_xpr, a00_ins],
    [0x000F4210, 0x80378800, 0x80385F90, [], {}, {}, {}],
    [0x0021D7D0, 0x8016F000, 0x801A76F0, [], {}, {}, {}],
]

dcall = [
]

cache = [
    [0x004E9FA0, 0x00577BC0], # anime
    [0x00590200, 0x00745F80], # audio
]
