def patch(data):
    return data

entry = 0x80246000
bss   = [0x8033A580, 0x0002CEE0]
sp    = 0x80200600
main  = 0x80246DF8
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
    "#define LIB_DYNRES\n"
    "#define GSP_F3D\n"
    "#define GSP_F3D_20D\n"
    "#define GSP_CACHE\n"
    "#define ASP_MAIN\n"
    "#define ASP_MAIN1\n"
    "#ifdef GEKKO\n"
    "#define APP_BORDER 8\n"
    "#else\n"
    "#define APP_BORDER 0\n"
    "#endif\n"
    "\n"
    "#define __osExceptionPreamble   0x80327640\n"
)

lib = {
    0x803223B0: "osSetTime",
    0x803223E0: "osMapTLB",
    0x803224A0: "osUnmapTLBAll",
    0x803224F0: "sprintf",
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
    0x80323A50: "sqrtf",
    0x80323A60: "osContStartReadData",
    0x80323B24: "osContGetReadData",
    0x80323CC0: "osContInit",
    0x80324080: "osEepromProbe",
    0x80324158: "__ull_div",
    0x80324194: "__ll_lshift",
    0x80324258: "__ll_mul",
    0x803243B0: "osInvalDCache",
    0x80324460: "osPiStartDma",
    0x80324570: "bzero",
    0x80324610: "osInvalICache",
    0x80324690: "osEepromLongRead",
    0x803247D0: "osEepromLongWrite",
    0x80324910: "bcopy",
    0x80324D74: "guOrtho",
    0x80325010: "guPerspective",
    0x80325070: "osGetTime",
    0x80325138: "__d_to_ull",
    0x803252A4: "__ull_to_d",
    0x80325310: "cosf",
    0x80325480: "sinf",
    0x80325688: "guTranslate",
    0x80325874: "guRotate",
    0x80325924: "guScale",
    0x80325970: "osAiSetFrequency",
    0x80325CD8: "alSeqFileNew",
    0x80325D20: "osWritebackDCache",
    0x80325DA0: "osAiGetLength",
    0x80325DB0: "osAiSetNextBuffer",
    0x80327EB0: "osVirtualToPhysical",
}

a00_pat = {
    # video_draw_cimg
    0x80247490: [0x35CE0000],
    0x802474A0: [0x371803C0],
    # video_cimg_clear
    0x802475A0: [0x3508C3BC],
    0x802475AC: [0x240A0000],
    # video_draw_border
    0x80247948: [0x1000001A, 0x00000000],
    # world_draw
    0x8027B46C: [0x37180000],
    0x8027B47C: [0x350803C0],
    0x8027B4F8: [0x35080000],
    0x8027B508: [0x354A03C0],
    0x8027B584: [0x37390000],
    0x8027B594: [0x352903C0],
    # gfx_draw_bg
    0x8027C99C: [0x3508C3BC],
    0x8027C9A8: [0x24180000],
}

a00_xpr = {
    0x80256F2C: "(int)video_l + 28", # credits str l
    0x80256F68: "(int)video_l + 28", # credits str l
    0x80256FA0: "(int)video_l + 28", # credits str l
    0x80256FF0: "(int)video_r - 28", # credits str r
    0x8027A904: "(int)video_l + 30 + 6*5", # "press" x
    0x8027A90C: "APP_BORDER + 12 + 18", # "press" y
    0x8027A918: "(int)video_l + 30 + 6*5", # "start" x
    0x8027A920: "APP_BORDER + 12", # "start" y
    0x8027B2D4: # transition fadeout radius
        "(int)(video_r-video_l)",
    0x8027B338: # transition fadein radius
        "(int)(video_r-video_l)",
}

a00_ins = {
    # obj cull
    0x8027D5C8: "    f18.f[IX] *= video_aspect;\n",
}

a02_pat = {
    # object spawn
    0x802C9C44: [0x3C018036, 0x24210C28, 0x1000000D, 0xAFA1001C],
}

a03_pat = {
    # hud clamp pos
    0x802D6B80: [0x00000000],
}

a03_xpr = {
    0x802CB900: "(int)video_l", # transition v0 x
    0x802CB944: "(int)video_r", # transition v1 x
    0x802CB988: "(int)video_r", # transition v2 x
    0x802CB9CC: "(int)video_l", # transition v3 x
    0x802CCDE0: "0x0080 + 0x0028", # reticle dl
    0x802CCE0C: # reticle v0 u
        "(int)(-3.6F * (video_r-video_l))",
    0x802CCE30: "(int)video_l", # reticle v0 x
    0x802CCE4C: # reticle v1 u
        "(int)( 3.6F * (video_r-video_l))",
    0x802CCE70: "(int)video_r", # reticle v1 x
    0x802CCE8C: # reticle v2 u
        "(int)( 3.6F * (video_r-video_l))",
    0x802CCEB0: "(int)video_r", # reticle v2 x
    0x802CCECC: # reticle v3 u
        "(int)(-3.6F * (video_r-video_l))",
    0x802CCEF0: "(int)video_l", # reticle v3 x
}

a03_ins = {
    # reticle border
    0x802CD19C:
        "{\n"
        "    int x = 0.15F * (video_r-video_l);\n"
        "    at = __read_s32((s16)0x007C + sp);\n"
        "    __write_u32((s16)0x007C + sp, at + 0x0028);\n"
        "    __write_u32((s16)0x0000 + at, 0xBA001402);\n"
        "    __write_u32((s16)0x0004 + at, 0x00300000);\n"
        "    __write_u32((s16)0x0008 + at, 0xF7000000);\n"
        "    __write_u32((s16)0x000C + at, 0x00010001);\n"
        "    __write_u32((s16)0x0010 + at, "
            "0xF60003BC | ((  x-1) & 0x3FF) << 14);\n"
        "    __write_u32((s16)0x0014 + at, 0x00000000);\n"
        "    __write_u32((s16)0x0018 + at, 0xF64FC3BC);\n"
        "    __write_u32((s16)0x001C + at, "
            "0x00000000 | ((320-x) & 0x3FF) << 14);\n"
        "    __write_u32((s16)0x0020 + at, 0xBA001402);\n"
        "    __write_u32((s16)0x0024 + at, 0x00000000);\n"
        "}\n",
}

a04_xpr = {
    0x802DB708: "(int)video_r - 30", # pause red coin
    0x802E3754: "(int)video_l + 22 + 16*0", # lives "," x
    0x802E375C: "240 - APP_BORDER - 7 - 16", # life "," y
    0x802E3768: "(int)video_l + 22 + 16*1", # life "*" x
    0x802E3770: "240 - APP_BORDER - 7 - 16", # life "*" y
    0x802E3784: "(int)video_l + 22 + 16*2", # life "%d" x
    0x802E378C: "240 - APP_BORDER - 7 - 16", # life "%d" y
    # 0x802E37B8: "(int)(320)/2 + 8 + 16*0", # coin "+" x
    0x802E37C0: "240 - APP_BORDER - 7 - 16", # coin "+" y
    # 0x802E37CC: "(int)(320)/2 + 8 + 16*1", # coin "*" x
    0x802E37D4: "240 - APP_BORDER - 7 - 16", # coin "*" y
    # 0x802E37E8: "(int)(320)/2 + 8 + 16*2", # coin "%d" x
    0x802E37F0: "240 - APP_BORDER - 7 - 16", # coin "%d" y
    0x802E386C: "(int)video_r - (22 + 16+16+12*2) + 16*0", # stars "-" x
    0x802E3874: "240 - APP_BORDER - 7 - 16", # star "-" y
    0x802E3890: "(int)video_r - (22 + 16+16+12*2) + 16*1", # stars "*" x
    0x802E3898: "240 - APP_BORDER - 7 - 16", # star "*" y
    0x802E38B8: # stars "%d" x
        "a0 + (int)video_r - (22 + 16+16+12*2) + 16*1",
    0x802E38C8: "240 - APP_BORDER - 7 - 16", # star "%d" y
    0x802E3914: "(int)video_r - (22 + 78)", # key "/" x
    # 0x802E391C: "", # key "/" y
    0x802E3A20: # time "time" x
        "(int)video_r - (22 + 12*1+3 + 9 + 12*2+1 + 10 + 10 + 12*5-1)",
    0x802E3A28: "240 - APP_BORDER - 7 - 16 - 8 - 16", # time "time" y
    0x802E3A34: # time "%0d" x
        "(int)video_r - (22 + 12*1+3 + 9 + 12*2+1 + 10 + 10)",
    0x802E3A38: "240 - APP_BORDER - 7 - 16 - 8 - 16", # time "%0d" y
    0x802E3A4C: # time "%02d" x
        "(int)video_r - (22 + 12*1+3 + 9 + 12*2+1)",
    0x802E3A50: "240 - APP_BORDER - 7 - 16 - 8 - 16", # time "%02d" y
    0x802E3A64: "(int)video_r - (22 + 12*1+3)", # time "%d" x
    0x802E3A68: "240 - APP_BORDER - 7 - 16 - 8 - 16", # time "%d" y
    0x802E3AAC: # time ' x
        "(int)video_r - (22 + 12*1+3 + 9 + 12*2+1 + 10)",
    0x802E3AB0: "APP_BORDER + 7 + 16 + 1", # time ' y
    0x802E3AC0: "(int)video_r - (22 + 12*1+3 + 9)", # time " x
    0x802E3AC4: "APP_BORDER + 7 + 16 + 1", # time " y
    0x802E3B58: "(int)video_r - (22 + 16+16)", # camera x
    0x802E3B60: "240 - APP_BORDER - 7 - 20", # camera y
}

a04_ins = {
    # pause
    0x802DB3C8: "    ARG_F(a1) = video_l;\n",
    0x802DB3E4:
        "    ARG_F(a1) = (1.0F/128) * (video_r-video_l);\n",
    # hud draw power
    0x802E3254: "    t8 += 8;\n",
}

a06_ins = {
    # audio sleep
    0x80317938: "    thread_yield(THREAD_YIELD_BREAK);\n",
}

segment = [
    [0x00001050, 0x80246050, 0x8027F590, [], a00_pat, a00_xpr, a00_ins],
    [0x0003A590, 0x8027F590, 0x8029C770, [], {}, {}, {}],
    [0x00057770, 0x8029C770, 0x802CB5C0, [], a02_pat, {}, {}],
    [0x000865C0, 0x802CB5C0, 0x802D5E00, [], a03_pat, a03_xpr, a03_ins],
    [0x00090E00, 0x802D5E00, 0x802F9730, [], {}, a04_xpr, a04_ins],
    [0x000B4730, 0x802F9730, 0x80314A30, [], {}, {}, {}],
    # 0x8032B260
    [0x000CFA30, 0x80314A30, 0x803223B0, [], {}, {}, a06_ins],
    [0x000F5580, 0x80378800, 0x80385F90, [], {}, {}, {}],
    [0x0021F4C0, 0x8016F000, 0x801A7830, [], {}, {}, {}],
]

dcall = [
]

cache = [
    [0x004EC000, 0x00579C20], # anime
    [0x00593560, 0x007B0860], # audio
]
