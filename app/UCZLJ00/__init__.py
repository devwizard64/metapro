import sys
import os
import struct

def slidec(data, i):
	sig, size = struct.unpack(">4sI8x", data[i:i+16])
	i += 16
	dst = B""
	bit = 0
	cnt = 0
	while len(dst) < size:
		if cnt == 0:
			bit, = struct.unpack(">B", data[i:i+1]); i += 1
			cnt = 8
		if bit & 0x80:
			dst += data[i:i+1]; i += 1
		else:
			x, = struct.unpack(">H", data[i:i+2]); i += 2
			o = len(dst) - ((x & 0x0FFF) + 1)
			n = x >> 12
			if n == 0:
				n, = struct.unpack(">B", data[i:i+1]); i += 1
				n += 18
			else:
				n += 2
			dst += (n*dst[o:o+n])[:n]
		bit <<= 1
		cnt -= 1
	return dst

def szs_decode(data, i):
	path = os.path.join("app", sys.argv[1], "%08X.bin" % i)
	if os.path.isfile(path):
		with open(path, "rb") as f: data = f.read()
	else:
		data = slidec(data, i)
		with open(path, "wb") as f: f.write(data)
	return data

def patch(data):
	data += szs_decode(data, 0x00A62840) # 0x02000000
	data += szs_decode(data, 0x00AFD890) # 0x02103D30
	dst = 0x80800000
	for i in range(0x7600, 0x7600+0x10*469, 0x10):
		zstart, = struct.unpack(">4x4xI4x", data[i:i+0x10])
		dlf = szs_decode(data, zstart)
		i, = struct.unpack(">I", dlf[-4:])
		stext, sbss = struct.unpack(">I4x4xI", dlf[-i:-i+0x10])
		if False: print("\t[0x%08X, 0x%08X, 0x%08X, [], {}, {}, {}]," % (
			len(data), dst, dst+stext
		))
		data += dlf
		dst += len(dlf)+sbss
	return data

entry = 0x80000400
bss   = [0x80006830, 0x00004910]
sp    = 0x80007220
main  = 0x80000498
reg = (
	1 <<  2 | # v0
	1 <<  3 | # v1
	1 <<  4 | # a0
	1 <<  5 | # a1
	1 <<  6 | # a2
	1 <<  7 | # a3
	1 << 16 | # s0
	1 << 29 | # sp
	1 << 32 | # f0
	1 << 38 | # f12
	1 << 39   # f14
)

header = (
	"#define EEPROM                  0\n"
	"#define SRAM\n"
	"#define AUDIO_FREQ              32000\n"
	"#define GSP_F3DEX\n"
	"#define GSP_F3DEX2\n"
	"#define GSP_SWVTX\n"
	"#define ASP_MAIN\n"
	"#define ASP_MAIN1\n"
	"\n"
	"#define __osExceptionPreamble   0x80002430\n"
	"#define __CartRomHandle         0x80009EA0\n"
	"#define __DriveRomHandle        0x800088C0\n"
)

lib = {
	0x80001600: "osDriveRomInit",
	0x80001E20: "osSendMesg",
	0x80002030: "osRecvMesg",
	0x8000219C: "__ull_rem",
	0x800021D8: "__ull_div",
	0x8000227C: "__ll_div",
	0x800022D8: "__ll_mul",
	0x80002D70: "osDestroyThread",
	0x80002F20: "osCreateThread",
	0x80003070: "osInitialize",
	0x80003440: "osWritebackDCache",
	0x800034C0: "osViGetCurrentFramebuffer",
	0x80003500: "osCreatePiManager",
	0x80003BE0: "osViBlack",
	0x80003CA0: "osGetThreadId",
	0x80003CC0: "osSetIntMask",
	0x80003D60: "osViSetMode",
	0x80003E90: "osGetMemSize",
	0x80003FB0: "osSetEventMesg",
	0x800040C0: "osEPiStartDma",
	0x800041A0: "osInvalICache",
	0x80004220: "osCreateMesgQueue",
	0x80004250: "osInvalDCache",
	0x80004480: "osSetThreadPri",
	0x800048C0: "osGetTime",
	0x80004D50: "osGetCount",
	0x80005130: "__osDisableInt",
	0x800051A0: "__osRestoreInt",
	0x80005630: "osEPiReadIo",
	0x80005680: "osCartRomInit",
	0x80005A70: "osSetTimer",
	0x80005BA0: "osCreateViManager",
	0x80005EC0: "osStartThread",

	0x800BBE80: "osAiSetNextBuffer",
	0x800CF7BC: "osSpTaskLoad",
	0x800CF94C: "osSpTaskStartGo",
	0x800CF990: "__osMotorAccess",
	0x800CFC04: "osMotorInit",
	0x800CFE20: "osContInit",
	0x800D0160: "osContStartReadData",
	0x800D01E4: "osContGetReadData",
	0x800D0570: "guPerspective",
	0x800D0710: "osSpTaskYield",
	0x800D0CD0: "osStopTimer",
	0x800D0DD0: "osAfterPreNMI",
	0x800D0DF0: "osContStartQuery",
	0x800D0E74: "osContGetQuery",
	0x800D2420: "osViSwapBuffer",
	0x800D2690: "osSpTaskYielded",
	0x800D2900: "osAiSetFrequency",
	0x800D2A50: "__osGetActiveQueue",
	0x800D2AF0: "osDpGetStatus",
	0x800D2B00: "osDpSetStatus",
	0x800D2C64: "guOrtho",
	0x800D2E40: "osViSetSpecialFeatures",
	0x800D3000: "osViSetEvent",
	0x800D3270: "osContSetCh",
	0x800D32E0: "osAiGetLength",
	0x800D5A80: "__osSpGetStatus",
	0x800D5A90: "__osSpSetStatus",
	0x800D5AA0: "osWritebackDCacheAll",
	0x800D5AD0: "__osGetCurrFaultedThread",
	0x800D5C84: "__ull_to_d",
	0x800D5CB8: "__ull_to_f",
	0x800D5CF0: "osViSetYScale",
	0x800D5D50: "osViGetCurrentFramebuffer",

	0x801C9B50: "osSetTime",
	0x801C9E28: "UCZLJ00_801C9E28",
	0x801C9EC0: "UCZLJ00_801C9EC0",
	0x801C9F90: "UCZLJ00_801C9F90",
	0x801C9FFC: "UCZLJ00_801C9FFC",
	0x801CA030: "UCZLJ00_801CA030",
	0x801CA070: "UCZLJ00_801CA070",
	0x801CA1F0: "UCZLJ00_801CA1F0",
	0x801CA740: "UCZLJ00_801CA740",
	0x801CAB04: "LeoReset",
	0x801CABB8: "LeoResetClear",
	0x801CBEF0: "LeoLBAToByte",
	0x801CC190: "UCZLJ00_801CC190",
	0x801CCCD0: "LeoSpdlMotor",
	0x801CE120: "LeoDriveExist",
	0x801CE630: "UCZLJ00_801CE630",
	0x801CE680: "LeoByteToLBA",
	0x801CF0B0: "UCZLJ00_801CF0B0",
	0x801CFBB0: "UCZLJ00_801CFBB0",
	0x801D2170: "LeoCJCreateLeoManager",
	0x801D2A80: "LeoCACreateLeoManager",
	0x801D2CB0: "UCZLJ00_801D2CB0",
}

a00_pat = {
	0x80001358: [0x00007825], # SP
	0x80001534: [0x00006025], # SP
	0x80001544: [0x00007025], # SP
}

a01_pat = {
	0x8001B174: [0x00000000], # z_effect_soft_sprite.c
	0x8001B4E0: [0x8C620008], # z_effect_soft_sprite.c
	0x8001EBFC: [0x00000000], # z_DLF.c
	0x800250F0: [0x00000000], # z_actor.c
	0x800251A4: [0x00001025], # z_actor.c
	0x800251B8: [0x8E030008], # z_actor.c
	0x800251CC: [0x8E020008], # z_actor.c
	0x800251DC: [0x8E020008], # z_actor.c
	0x80066E10: [0x3C02800F, 0x8C421C04], # z_map_mark.c
	0x80099C74: [0x8E0E000C], # KaleidoArea_allocp
	0x80099D54: [0x00001025], # KaleidoArea_allocp
	0x800CCD50: [0x00C01025], # loadfragment2.c
}

segment = [
	# boot
	[0x00001060, 0x80000460, 0x80001600, [], a00_pat, {}, {}],
	[0x000023E0, 0x800017E0, 0x80001D60, [], {}, {}, {}],
	[0x00003A80, 0x80002E80, 0x80002F20, [], {}, {}, {}],
	[0x000059C0, 0x80004DC0, 0x800050D0, [], {}, {}, {}],
	# code
	[0x02000000, 0x800110A0, 0x800AF828, [], a01_pat, {}, {}],
	[0x0209F1E0, 0x800B0280, 0x800BBE80, [], {}, {}, {}],
	[0x020AAE60, 0x800BBF00, 0x800CF370, [], a01_pat, {}, {}],
	[0x020BE330, 0x800CF3D0, 0x800CF6A0, [], {}, {}, {}],
	[0x020BF690, 0x800D0730, 0x800D08E8, [], {}, {}, {}],
	[0x020BF900, 0x800D09A0, 0x800D0CD0, [], {}, {}, {}],
	[0x020BFD20, 0x800D0DC0, 0x800D0DD0, [], {}, {}, {}],
	[0x020BFE00, 0x800D0EA0, 0x800D23F0, [], {}, {}, {}],
	[0x020C13D0, 0x800D2470, 0x800D2690, [], {}, {}, {}],
	[0x020C1670, 0x800D2710, 0x800D2900, [], {}, {}, {}],
	[0x020C19C0, 0x800D2A60, 0x800D2AF0, [], {}, {}, {}],
	[0x020C1C30, 0x800D2CD0, 0x800D2E40, [], {}, {}, {}],
	[0x020C1F00, 0x800D2FA0, 0x800D2FD0, [], {}, {}, {}],
	[0x020C1FD0, 0x800D3070, 0x800D31F0, [], {}, {}, {}],
	[0x020C2298, 0x800D3338, 0x800D3390, [], {}, {}, {}],
	[0x020C32F0, 0x800D4390, 0x800D44B0, [], {}, {}, {}],
	[0x020C3AF0, 0x800D4B90, 0x800D5A80, [], {}, {}, {}],
	[0x020C4D30, 0x800D5DD0, 0x800E2FC0, [], {}, {}, {}],
	# leo
	[0x02103D30, 0x801C6E80, 0x801C9B50, [], {}, {}, {}],
	# 0x801D2040

	[0x02116A40, 0x80800000, 0x80800910, [], {}, {}, {}],
	[0x02117400, 0x808009C0, 0x80801D90, [], {}, {}, {}],
	[0x0211A160, 0x80803720, 0x80803850, [], {}, {}, {}],
	[0x0211A2C0, 0x80803880, 0x808124D0, [], {}, {}, {}],
	[0x0212A1E0, 0x808137C0, 0x80829350, [], {}, {}, {}],
	[0x02146B70, 0x808301C0, 0x808512D0, [], {}, {}, {}],
	[0x0216D0D0, 0x808567F0, 0x808567F0, [], {}, {}, {}],
	[0x02173C30, 0x8085D350, 0x80862240, [], {}, {}, {}],
	[0x021794E0, 0x80862C00, 0x808649B0, [], {}, {}, {}],
	[0x0217BE00, 0x80865520, 0x80866870, [], {}, {}, {}],
	[0x0217D410, 0x80866B30, 0x808677C0, [], {}, {}, {}],
	[0x0217E200, 0x80867920, 0x808685D0, [], {}, {}, {}],
	[0x0217F030, 0x80868750, 0x8086A080, [], {}, {}, {}],
	[0x02180B60, 0x8086A290, 0x8086DEB0, [], {}, {}, {}],
	[0x02184CF0, 0x8086E420, 0x808705B0, [], {}, {}, {}],
	[0x021872D0, 0x80870A00, 0x80871710, [], {}, {}, {}],
	[0x021881A0, 0x808718D0, 0x80873060, [], {}, {}, {}],
	[0x02189BB0, 0x808732E0, 0x80875870, [], {}, {}, {}],
	[0x0218C950, 0x80876080, 0x80877EF0, [], {}, {}, {}],
	[0x0218EAC0, 0x808781F0, 0x80882D40, [], {}, {}, {}],
	[0x0219ACE0, 0x80884410, 0x808857D0, [], {}, {}, {}],
	[0x0219C3D0, 0x80885B00, 0x80889E70, [], {}, {}, {}],
	[0x021A0D90, 0x8088A4C0, 0x8088D220, [], {}, {}, {}],
	[0x021A40C0, 0x8088D7F0, 0x808902C0, [], {}, {}, {}],
	[0x021A6E60, 0x80890590, 0x80891CF0, [], {}, {}, {}],
	[0x021A88D0, 0x80892000, 0x80895220, [], {}, {}, {}],
	[0x021ABFD0, 0x80895700, 0x80896500, [], {}, {}, {}],
	[0x021ACFA0, 0x808966D0, 0x808967A0, [], {}, {}, {}],
	[0x021AD0D0, 0x80896800, 0x8089C880, [], {}, {}, {}],
	[0x021B3BC0, 0x8089D300, 0x8089D7B0, [], {}, {}, {}],
	[0x021B4150, 0x8089D890, 0x808A3670, [], {}, {}, {}],
	[0x021BDC30, 0x808A7370, 0x808AC710, [], {}, {}, {}],
	[0x021C3BB0, 0x808AD2F0, 0x808AED60, [], {}, {}, {}],
	[0x021C79B0, 0x808B10F0, 0x808B3C50, [], {}, {}, {}],
	[0x021CA860, 0x808B3FC0, 0x808B67D0, [], {}, {}, {}],
	[0x021CD4F0, 0x808B6C50, 0x808B6EC0, [], {}, {}, {}],
	[0x021CD7F0, 0x808B6F50, 0x808B8160, [], {}, {}, {}],
	[0x021CEC10, 0x808B8370, 0x808BA180, [], {}, {}, {}],
	[0x021D0E90, 0x808BA5F0, 0x808BC0D0, [], {}, {}, {}],
	[0x021D2D30, 0x808BC490, 0x808BCF10, [], {}, {}, {}],
	[0x021D38C0, 0x808BD020, 0x808BD7A0, [], {}, {}, {}],
	[0x021D4180, 0x808BD8E0, 0x808BF910, [], {}, {}, {}],
	[0x021D68E0, 0x808C0080, 0x808C1DF0, [], {}, {}, {}],
	[0x021D8BB0, 0x808C2350, 0x808C3F80, [], {}, {}, {}],
	[0x021DAA00, 0x808C41A0, 0x808C6840, [], {}, {}, {}],
	[0x021DD670, 0x808C6E10, 0x808C9B20, [], {}, {}, {}],
	[0x021E09D0, 0x808CA170, 0x808CBB20, [], {}, {}, {}],
	[0x021E2630, 0x808CBDD0, 0x808CC680, [], {}, {}, {}],
	[0x021E2FC0, 0x808CC760, 0x808CE860, [], {}, {}, {}],
	[0x021E55E0, 0x808CED80, 0x808D4560, [], {}, {}, {}],
	[0x021EBBC0, 0x808D5360, 0x808D5C40, [], {}, {}, {}],
	[0x021ED210, 0x808D69B0, 0x808D74A0, [], {}, {}, {}],
	[0x021EDF50, 0x808D7760, 0x808D7DD0, [], {}, {}, {}],
	[0x021EE7A0, 0x808D7FB0, 0x808D8680, [], {}, {}, {}],
	[0x021EEFD0, 0x808D87E0, 0x808D9320, [], {}, {}, {}],
	[0x021EFD50, 0x808D9560, 0x808DA420, [], {}, {}, {}],
	[0x021F0E40, 0x808DA650, 0x808DB060, [], {}, {}, {}],
	[0x021F1A20, 0x808DB230, 0x808DC490, [], {}, {}, {}],
	[0x021F2E70, 0x808DC680, 0x808DD3F0, [], {}, {}, {}],
	[0x021F3D90, 0x808DD5A0, 0x808DD950, [], {}, {}, {}],
	[0x021F41F0, 0x808DDA00, 0x808E3490, [], {}, {}, {}],
	[0x021FA950, 0x808E4190, 0x808E4AF0, [], {}, {}, {}],
	[0x021FB3F0, 0x808E4C30, 0x808E5BE0, [], {}, {}, {}],
	[0x021FC500, 0x808E5D40, 0x808E9810, [], {}, {}, {}],
	[0x02200640, 0x808E9E80, 0x808EB120, [], {}, {}, {}],
	[0x02201AB0, 0x808EB2F0, 0x808EB920, [], {}, {}, {}],
	[0x02202210, 0x808EBA50, 0x808EE4C0, [], {}, {}, {}],
	[0x02204F60, 0x808EE7A0, 0x808F2680, [], {}, {}, {}],
	[0x02209690, 0x808F2ED0, 0x808F32F0, [], {}, {}, {}],
	[0x02209B60, 0x808F33A0, 0x808F3420, [], {}, {}, {}],
	[0x02209C40, 0x808F3480, 0x808F3B50, [], {}, {}, {}],
	[0x0220A3F0, 0x808F3C30, 0x808F41E0, [], {}, {}, {}],
	[0x0220AAD0, 0x808F4310, 0x808F8700, [], {}, {}, {}],
	[0x0220F840, 0x808F9080, 0x808FB0A0, [], {}, {}, {}],
	[0x02211C40, 0x808FB480, 0x808FEA20, [], {}, {}, {}],
	[0x022156E0, 0x808FEF20, 0x808FF040, [], {}, {}, {}],
	[0x02215880, 0x808FF0C0, 0x80900490, [], {}, {}, {}],
	[0x02216E70, 0x809006B0, 0x80900C70, [], {}, {}, {}],
	[0x022174C0, 0x80900D00, 0x80901840, [], {}, {}, {}],
	[0x02218330, 0x80901B70, 0x809024A0, [], {}, {}, {}],
	[0x02219900, 0x80903140, 0x80903A40, [], {}, {}, {}],
	[0x0221A3F0, 0x80903C30, 0x809043D0, [], {}, {}, {}],
	[0x0221ACE0, 0x80904520, 0x80908220, [], {}, {}, {}],
	[0x0221EF80, 0x809087D0, 0x809092D0, [], {}, {}, {}],
	[0x0221FBB0, 0x80909410, 0x80909790, [], {}, {}, {}],
	[0x0221FFA0, 0x80909800, 0x8090ADF0, [], {}, {}, {}],
	[0x022217A0, 0x8090B000, 0x8090B130, [], {}, {}, {}],
	[0x02221930, 0x8090B190, 0x8090B560, [], {}, {}, {}],
	[0x02221DC0, 0x8090B630, 0x8090D770, [], {}, {}, {}],
	[0x02224460, 0x8090DCD0, 0x8090EBA0, [], {}, {}, {}],
	[0x022255F0, 0x8090EE70, 0x8090F960, [], {}, {}, {}],
	[0x022262C0, 0x8090FB40, 0x809106E0, [], {}, {}, {}],
	[0x02227020, 0x809108A0, 0x80912E70, [], {}, {}, {}],
	[0x02229950, 0x809131D0, 0x80913D10, [], {}, {}, {}],
	[0x0222A620, 0x80913EA0, 0x80917610, [], {}, {}, {}],
	[0x0222E2F0, 0x80917B70, 0x80918840, [], {}, {}, {}],
	[0x0222F090, 0x80918910, 0x80918E30, [], {}, {}, {}],
	[0x0222F6A0, 0x80918F20, 0x809193B0, [], {}, {}, {}],
	[0x02230CF0, 0x8091A570, 0x8091C920, [], {}, {}, {}],
	[0x02233310, 0x8091CB90, 0x8091CED0, [], {}, {}, {}],
	[0x022336F0, 0x8091CF70, 0x8091D3D0, [], {}, {}, {}],
	[0x02233C70, 0x8091D4F0, 0x8091D5F0, [], {}, {}, {}],
	[0x02233DD0, 0x8091D650, 0x8091E2B0, [], {}, {}, {}],
	[0x02234C60, 0x8091E4E0, 0x8091E910, [], {}, {}, {}],
	[0x02235120, 0x8091E9A0, 0x8091F8F0, [], {}, {}, {}],
	[0x022362F0, 0x8091FB80, 0x8091FFA0, [], {}, {}, {}],
	[0x02236770, 0x80920000, 0x809202F0, [], {}, {}, {}],
	[0x02236AB0, 0x80920340, 0x80921500, [], {}, {}, {}],
	[0x02237D50, 0x809215E0, 0x809216E0, [], {}, {}, {}],
	[0x02237EA0, 0x80921730, 0x809217E0, [], {}, {}, {}],
	[0x02237FA0, 0x80921830, 0x80924BB0, [], {}, {}, {}],
	[0x0223B960, 0x809251F0, 0x80926D00, [], {}, {}, {}],
	[0x0223D790, 0x80927020, 0x80927820, [], {}, {}, {}],
	[0x0223E0C0, 0x80927950, 0x809282B0, [], {}, {}, {}],
	[0x0223EBB0, 0x80928440, 0x809289E0, [], {}, {}, {}],
	[0x0223F210, 0x80928AA0, 0x80929290, [], {}, {}, {}],
	[0x0223FB80, 0x80929410, 0x8092A950, [], {}, {}, {}],
	[0x02241430, 0x8092ACC0, 0x80930190, [], {}, {}, {}],
	[0x02246F30, 0x809307C0, 0x809330A0, [], {}, {}, {}],
	[0x0224ABE0, 0x809344C0, 0x80935070, [], {}, {}, {}],
	[0x0224B8C0, 0x809351A0, 0x809380C0, [], {}, {}, {}],
	[0x0224ECA0, 0x80938580, 0x80939760, [], {}, {}, {}],
	[0x022501B0, 0x80939A90, 0x8093BF40, [], {}, {}, {}],
	[0x02252A60, 0x8093C340, 0x80940A70, [], {}, {}, {}],
	[0x02257750, 0x80941030, 0x809426B0, [], {}, {}, {}],
	[0x02259040, 0x80942920, 0x809443A0, [], {}, {}, {}],
	[0x0225AE70, 0x80944760, 0x80945590, [], {}, {}, {}],
	[0x0225BE80, 0x80945770, 0x80948960, [], {}, {}, {}],
	[0x0225F5F0, 0x80948F60, 0x8094F5A0, [], {}, {}, {}],
	[0x02266920, 0x80950290, 0x80952F60, [], {}, {}, {}],
	[0x02269B40, 0x809534B0, 0x80954AA0, [], {}, {}, {}],
	[0x0226B5E0, 0x80954F50, 0x80957620, [], {}, {}, {}],
	[0x0226E2A0, 0x80957C10, 0x809596D0, [], {}, {}, {}],
	[0x022700A0, 0x80959A10, 0x80959F90, [], {}, {}, {}],
	[0x02270710, 0x8095A080, 0x8095B210, [], {}, {}, {}],
	[0x02271A60, 0x8095B3D0, 0x8095B9D0, [], {}, {}, {}],
	[0x02272120, 0x8095BA90, 0x8095C110, [], {}, {}, {}],
	[0x02273E20, 0x8095D790, 0x8095E290, [], {}, {}, {}],
	[0x022760F0, 0x8095FA60, 0x809655F0, [], {}, {}, {}],
	[0x0227D790, 0x80967100, 0x8096A810, [], {}, {}, {}],
	[0x022814C0, 0x8096AE30, 0x8096B950, [], {}, {}, {}],
	[0x022821D0, 0x8096BB40, 0x8096D380, [], {}, {}, {}],
	[0x02283CA0, 0x8096D610, 0x8096E3A0, [], {}, {}, {}],
	[0x02284C70, 0x8096E5E0, 0x8096F360, [], {}, {}, {}],
	[0x02285B50, 0x8096F4C0, 0x8096FF70, [], {}, {}, {}],
	[0x022866B0, 0x80970020, 0x80972670, [], {}, {}, {}],
	[0x02289E90, 0x80973800, 0x80976370, [], {}, {}, {}],
	[0x0228DE00, 0x80977770, 0x80977DC0, [], {}, {}, {}],
	[0x0228E510, 0x80977E80, 0x80979400, [], {}, {}, {}],
	[0x0228FCC0, 0x80979630, 0x80979C80, [], {}, {}, {}],
	[0x022903D0, 0x80979D40, 0x8097AD30, [], {}, {}, {}],
	[0x02291570, 0x8097AEE0, 0x8097B1E0, [], {}, {}, {}],
	[0x02291970, 0x8097B2E0, 0x8097B890, [], {}, {}, {}],
	[0x02292030, 0x8097B9A0, 0x8097C2C0, [], {}, {}, {}],
	[0x02292A70, 0x8097C3E0, 0x8097CD40, [], {}, {}, {}],
	[0x02293690, 0x8097D000, 0x8097EF00, [], {}, {}, {}],
	[0x02295890, 0x8097F200, 0x80980310, [], {}, {}, {}],
	[0x02296AC0, 0x80980430, 0x80981340, [], {}, {}, {}],
	[0x02297BD0, 0x80981540, 0x80982490, [], {}, {}, {}],
	[0x02298CC0, 0x80982630, 0x80983BD0, [], {}, {}, {}],
	[0x0229A580, 0x80983EF0, 0x80984310, [], {}, {}, {}],
	[0x0229AA90, 0x80984400, 0x80984B50, [], {}, {}, {}],
	[0x0229B360, 0x80984CD0, 0x80991760, [], {}, {}, {}],
	[0x022A9B50, 0x8099BEC0, 0x8099C760, [], {}, {}, {}],
	[0x022AA570, 0x8099C8E0, 0x8099DB90, [], {}, {}, {}],
	[0x022ABB40, 0x8099DEB0, 0x8099E830, [], {}, {}, {}],
	[0x022AC5E0, 0x8099E950, 0x8099F9A0, [], {}, {}, {}],
	[0x022AD7D0, 0x8099FB40, 0x8099FD00, [], {}, {}, {}],
	[0x022ADA20, 0x8099FD90, 0x809A07E0, [], {}, {}, {}],
	[0x022AE7C0, 0x809A0B30, 0x809A17B0, [], {}, {}, {}],
	[0x022AF580, 0x809A18F0, 0x809A1AF0, [], {}, {}, {}],
	[0x022AF820, 0x809A1B90, 0x809A4FE0, [], {}, {}, {}],
	[0x022B3DF0, 0x809A6160, 0x809B0040, [], {}, {}, {}],
	[0x022BF390, 0x809B6210, 0x809B73D0, [], {}, {}, {}],
	[0x022C07D0, 0x809B7650, 0x809B9D10, [], {}, {}, {}],
	[0x022C32E0, 0x809BA160, 0x809BB770, [], {}, {}, {}],
	[0x022C4BD0, 0x809BBA50, 0x809BCAF0, [], {}, {}, {}],
	[0x022C5EA0, 0x809BCD20, 0x809BE8B0, [], {}, {}, {}],
	[0x022C89C0, 0x809BF840, 0x809C0430, [], {}, {}, {}],
	[0x022C9720, 0x809C05A0, 0x809C2D80, [], {}, {}, {}],
	[0x022CC480, 0x809C3300, 0x809C4930, [], {}, {}, {}],
	[0x022CDD80, 0x809C4C00, 0x809C5CD0, [], {}, {}, {}],
	[0x022CF2C0, 0x809C6140, 0x809C6EB0, [], {}, {}, {}],
	[0x022D0190, 0x809C7010, 0x809C7730, [], {}, {}, {}],
	[0x022D0A50, 0x809C78D0, 0x809C9380, [], {}, {}, {}],
	[0x022D37D0, 0x809CA650, 0x809CABE0, [], {}, {}, {}],
	[0x022D3E10, 0x809CAC90, 0x809CADF0, [], {}, {}, {}],
	[0x022D3FE0, 0x809CAE60, 0x809CBFF0, [], {}, {}, {}],
	[0x022D53F0, 0x809CC270, 0x809CCF00, [], {}, {}, {}],
	[0x022D6330, 0x809CD1B0, 0x809CD8E0, [], {}, {}, {}],
	[0x022D6B10, 0x809CD990, 0x809CE7B0, [], {}, {}, {}],
	[0x022D7B70, 0x809CE9F0, 0x809CFEA0, [], {}, {}, {}],
	[0x022D9210, 0x809D0090, 0x809D0F60, [], {}, {}, {}],
	[0x022DA2E0, 0x809D1160, 0x809E27A0, [], {}, {}, {}],
	[0x022ED360, 0x809E6C60, 0x809E8D80, [], {}, {}, {}],
	[0x022EF880, 0x809E9180, 0x809EAE50, [], {}, {}, {}],
	[0x022F1750, 0x809EB050, 0x809EB980, [], {}, {}, {}],
	[0x022F2240, 0x809EBB40, 0x809ECC10, [], {}, {}, {}],
	[0x022F34F0, 0x809ECDF0, 0x809ED9D0, [], {}, {}, {}],
	[0x022F42B0, 0x809EDBB0, 0x809EE340, [], {}, {}, {}],
	[0x022F4B70, 0x809EE470, 0x809EE920, [], {}, {}, {}],
	[0x022F50F0, 0x809EEA00, 0x809EEF70, [], {}, {}, {}],
	[0x022F5730, 0x809EF040, 0x809EF540, [], {}, {}, {}],
	[0x022F5CE0, 0x809EF5F0, 0x809F0640, [], {}, {}, {}],
	[0x022F7110, 0x809F0A20, 0x809F19B0, [], {}, {}, {}],
	[0x022F83F0, 0x809F1D00, 0x809FFD50, [], {}, {}, {}],
	[0x0231A660, 0x80A17AF0, 0x80A21E20, [], {}, {}, {}],
	[0x02326B80, 0x80A24050, 0x80A25710, [], {}, {}, {}],
	[0x023284B0, 0x80A25980, 0x80A27E30, [], {}, {}, {}],
	[0x0232AF40, 0x80A28410, 0x80A28C60, [], {}, {}, {}],
	[0x0232B950, 0x80A28E20, 0x80A29D50, [], {}, {}, {}],
	[0x0232CB80, 0x80A2A050, 0x80A2AFE0, [], {}, {}, {}],
	[0x0232DD70, 0x80A2B240, 0x80A2B910, [], {}, {}, {}],
	[0x0232E540, 0x80A2BA10, 0x80A2C680, [], {}, {}, {}],
	[0x0232FD90, 0x80A2D260, 0x80A2F9D0, [], {}, {}, {}],
	[0x02332AA0, 0x80A2FF70, 0x80A301E0, [], {}, {}, {}],
	[0x02332D70, 0x80A30240, 0x80A310B0, [], {}, {}, {}],
	[0x02333DF0, 0x80A312D0, 0x80A31580, [], {}, {}, {}],
	[0x02334130, 0x80A31610, 0x80A31C20, [], {}, {}, {}],
	[0x023348B0, 0x80A31D90, 0x80A322B0, [], {}, {}, {}],
	[0x02334F60, 0x80A32440, 0x80A33C70, [], {}, {}, {}],
	[0x02336C80, 0x80A34160, 0x80A34480, [], {}, {}, {}],
	[0x02337030, 0x80A34510, 0x80A44A10, [], {}, {}, {}],
	[0x0234AA00, 0x80A4EFC0, 0x80A506A0, [], {}, {}, {}],
	[0x0234C4B0, 0x80A50A70, 0x80A50E50, [], {}, {}, {}],
	[0x0234C930, 0x80A50EF0, 0x80A52110, [], {}, {}, {}],
	[0x0234DD10, 0x80A522D0, 0x80A524E0, [], {}, {}, {}],
	[0x0234DFB0, 0x80A52570, 0x80A53060, [], {}, {}, {}],
	[0x0234EBB0, 0x80A53170, 0x80A533F0, [], {}, {}, {}],
	[0x0234EEC0, 0x80A53480, 0x80A53CD0, [], {}, {}, {}],
	[0x0234F810, 0x80A53DD0, 0x80A54E40, [], {}, {}, {}],
	[0x02351E50, 0x80A56410, 0x80A57020, [], {}, {}, {}],
	[0x02352C30, 0x80A571F0, 0x80A57320, [], {}, {}, {}],
	[0x02352DC0, 0x80A57380, 0x80A593B0, [], {}, {}, {}],
	[0x023552E0, 0x80A598A0, 0x80A5AAF0, [], {}, {}, {}],
	[0x023568B0, 0x80A5AE70, 0x80A5CBC0, [], {}, {}, {}],
	[0x023589C0, 0x80A5CF80, 0x80A5DC70, [], {}, {}, {}],
	[0x02359A00, 0x80A5DFC0, 0x80A5E660, [], {}, {}, {}],
	[0x0235A2D0, 0x80A5E890, 0x80A5F1F0, [], {}, {}, {}],
	[0x0235C190, 0x80A60750, 0x80A610D0, [], {}, {}, {}],
	[0x0235E070, 0x80A62630, 0x80A62FC0, [], {}, {}, {}],
	[0x0235FF60, 0x80A64520, 0x80A650C0, [], {}, {}, {}],
	[0x02360C60, 0x80A65220, 0x80A66000, [], {}, {}, {}],
	[0x02361C50, 0x80A66210, 0x80A66C80, [], {}, {}, {}],
	[0x023628A0, 0x80A66F40, 0x80A6ADA0, [], {}, {}, {}],
	[0x02366ED0, 0x80A6B580, 0x80A6C840, [], {}, {}, {}],
	[0x023683E0, 0x80A6CA90, 0x80A6FC70, [], {}, {}, {}],
	[0x0236BD10, 0x80A703D0, 0x80A70640, [], {}, {}, {}],
	[0x0236C020, 0x80A706E0, 0x80A71CF0, [], {}, {}, {}],
	[0x0236D850, 0x80A71F10, 0x80A72790, [], {}, {}, {}],
	[0x0236E190, 0x80A72850, 0x80A72C40, [], {}, {}, {}],
	[0x023705A0, 0x80A74C60, 0x80A75CE0, [], {}, {}, {}],
	[0x02371930, 0x80A75FF0, 0x80A76510, [], {}, {}, {}],
	[0x02371F20, 0x80A765E0, 0x80A77890, [], {}, {}, {}],
	[0x02373470, 0x80A77B30, 0x80A786C0, [], {}, {}, {}],
	[0x02374110, 0x80A787D0, 0x80A79040, [], {}, {}, {}],
	[0x02374AC0, 0x80A79180, 0x80A799F0, [], {}, {}, {}],
	[0x023753B0, 0x80A79A70, 0x80A79FC0, [], {}, {}, {}],
	[0x02375970, 0x80A7A030, 0x80A7BD80, [], {}, {}, {}],
	[0x02377980, 0x80A7C040, 0x80A7D4A0, [], {}, {}, {}],
	[0x02379090, 0x80A7D750, 0x80A7DE00, [], {}, {}, {}],
	[0x02379840, 0x80A7DF00, 0x80A7F6E0, [], {}, {}, {}],
	[0x0237B1F0, 0x80A7F8B0, 0x80A80AC0, [], {}, {}, {}],
	[0x0237C6D0, 0x80A80D90, 0x80A83090, [], {}, {}, {}],
	[0x0237EE60, 0x80A83520, 0x80A839B0, [], {}, {}, {}],
	[0x0237F3D0, 0x80A83A90, 0x80A85390, [], {}, {}, {}],
	[0x02381190, 0x80A85850, 0x80A85B50, [], {}, {}, {}],
	[0x02381550, 0x80A85C10, 0x80A864C0, [], {}, {}, {}],
	[0x02381F70, 0x80A86630, 0x80A86AF0, [], {}, {}, {}],
	[0x02382540, 0x80A86C00, 0x80A876C0, [], {}, {}, {}],
	[0x02383A20, 0x80A880E0, 0x80A885C0, [], {}, {}, {}],
	[0x023840F0, 0x80A887B0, 0x80A8A850, [], {}, {}, {}],
	[0x023864B0, 0x80A8AB70, 0x80A8BA10, [], {}, {}, {}],
	[0x02387570, 0x80A8BC30, 0x80A8C6C0, [], {}, {}, {}],
	[0x02388200, 0x80A8C8C0, 0x80A8DD80, [], {}, {}, {}],
	[0x02389940, 0x80A8E000, 0x80A8E950, [], {}, {}, {}],
	[0x0238A380, 0x80A8EA40, 0x80A8EEC0, [], {}, {}, {}],
	[0x0238A8E0, 0x80A8EFA0, 0x80A8F0B0, [], {}, {}, {}],
	[0x0238AA40, 0x80A8F100, 0x80A90E10, [], {}, {}, {}],
	[0x0238CA70, 0x80A91130, 0x80A91280, [], {}, {}, {}],
	[0x0238CC10, 0x80A912D0, 0x80A92490, [], {}, {}, {}],
	[0x0238E250, 0x80A929D0, 0x80A94F40, [], {}, {}, {}],
	[0x02396DC0, 0x80A9DF70, 0x80A9F550, [], {}, {}, {}],
	[0x023986A0, 0x80A9F850, 0x80AA0430, [], {}, {}, {}],
	[0x023993D0, 0x80AA0580, 0x80AA0B60, [], {}, {}, {}],
	[0x02399AC0, 0x80AA0C70, 0x80AA1680, [], {}, {}, {}],
	[0x0239A660, 0x80AA1810, 0x80AA1B30, [], {}, {}, {}],
	[0x0239A9F0, 0x80AA1BA0, 0x80AA3F60, [], {}, {}, {}],
	[0x0239DB40, 0x80AA4CF0, 0x80AA55E0, [], {}, {}, {}],
	[0x0239E510, 0x80AA56C0, 0x80AA73C0, [], {}, {}, {}],
	[0x023A05A0, 0x80AA7750, 0x80AA87B0, [], {}, {}, {}],
	[0x023A1800, 0x80AA89B0, 0x80AA8B50, [], {}, {}, {}],
	[0x023A1A10, 0x80AA8BC0, 0x80AAA8B0, [], {}, {}, {}],
	[0x023A3C80, 0x80AAAE30, 0x80AAB400, [], {}, {}, {}],
	[0x023A4310, 0x80AAB4C0, 0x80AAB920, [], {}, {}, {}],
	[0x023A4810, 0x80AAB9C0, 0x80AAC4D0, [], {}, {}, {}],
	[0x023A5430, 0x80AAC5E0, 0x80AAD300, [], {}, {}, {}],
	[0x023A62C0, 0x80AAD470, 0x80AAE770, [], {}, {}, {}],
	[0x023A7800, 0x80AAE9B0, 0x80AAF240, [], {}, {}, {}],
	[0x023A8170, 0x80AAF320, 0x80AB2920, [], {}, {}, {}],
	[0x023ABD10, 0x80AB2EC0, 0x80AB4410, [], {}, {}, {}],
	[0x023B4E60, 0x80ABC010, 0x80ABC210, [], {}, {}, {}],
	[0x023B5170, 0x80ABC320, 0x80ABD5B0, [], {}, {}, {}],
	[0x023B65A0, 0x80ABD750, 0x80ABDFE0, [], {}, {}, {}],
	[0x023B6FC0, 0x80ABE170, 0x80AC1660, [], {}, {}, {}],
	[0x023BB600, 0x80AC27B0, 0x80AC3390, [], {}, {}, {}],
	[0x023BC340, 0x80AC3500, 0x80AC3D10, [], {}, {}, {}],
	[0x023BCD20, 0x80AC3EE0, 0x80AC4D00, [], {}, {}, {}],
	[0x023BDF00, 0x80AC50C0, 0x80AC5530, [], {}, {}, {}],
	[0x023BE450, 0x80AC5610, 0x80AC5D00, [], {}, {}, {}],
	[0x023BEC90, 0x80AC5E50, 0x80AC6840, [], {}, {}, {}],
	[0x023BF7C0, 0x80AC6980, 0x80AC6C70, [], {}, {}, {}],
	[0x023BFB50, 0x80AC6D10, 0x80AC71F0, [], {}, {}, {}],
	[0x023C0110, 0x80AC72D0, 0x80AC8100, [], {}, {}, {}],
	[0x023C1100, 0x80AC82C0, 0x80AC85C0, [], {}, {}, {}],
	[0x023C8100, 0x80ACF2C0, 0x80ACFF20, [], {}, {}, {}],
	[0x023C8EB0, 0x80AD0070, 0x80AD0550, [], {}, {}, {}],
	[0x023C9440, 0x80AD0600, 0x80AD1900, [], {}, {}, {}],
	[0x023CAA60, 0x80AD1C20, 0x80AD4EF0, [], {}, {}, {}],
	[0x023CEBA0, 0x80AD5D60, 0x80AD7010, [], {}, {}, {}],
	[0x023D00B0, 0x80AD7270, 0x80AD7F90, [], {}, {}, {}],
	[0x023D0FA0, 0x80AD8160, 0x80AD8630, [], {}, {}, {}],
	[0x023D1500, 0x80AD86C0, 0x80AD9260, [], {}, {}, {}],
	[0x023D2270, 0x80AD9430, 0x80ADA260, [], {}, {}, {}],
	[0x023D33E0, 0x80ADA5A0, 0x80ADB160, [], {}, {}, {}],
	[0x023D4190, 0x80ADB350, 0x80ADBC10, [], {}, {}, {}],
	[0x023D4B60, 0x80ADBD20, 0x80ADCFB0, [], {}, {}, {}],
	[0x023D9920, 0x80AE1640, 0x80AE2620, [], {}, {}, {}],
	[0x023DAB50, 0x80AE2870, 0x80AE4900, [], {}, {}, {}],
	[0x023DD1C0, 0x80AE4EE0, 0x80AE7760, [], {}, {}, {}],
	[0x023E0B00, 0x80AE8820, 0x80AE9790, [], {}, {}, {}],
	[0x023E4D20, 0x80AECA40, 0x80AECD90, [], {}, {}, {}],
	[0x023E50D0, 0x80AECDF0, 0x80AEFB20, [], {}, {}, {}],
	[0x023E7FE0, 0x80AF1D00, 0x80AF2240, [], {}, {}, {}],
	[0x023E85D0, 0x80AF22F0, 0x80AF2690, [], {}, {}, {}],
	[0x023E8A40, 0x80AF2760, 0x80AF73F0, [], {}, {}, {}],
	[0x023EE040, 0x80AF7D60, 0x80AFB230, [], {}, {}, {}],
	[0x023F1A20, 0x80AFB7D0, 0x80AFC1F0, [], {}, {}, {}],
	[0x023F2500, 0x80AFC2B0, 0x80AFC660, [], {}, {}, {}],
	[0x023F2970, 0x80AFC720, 0x80AFD4B0, [], {}, {}, {}],
	[0x023F3870, 0x80AFD620, 0x80B046F0, [], {}, {}, {}],
	[0x023FB6C0, 0x80B05470, 0x80B0EEF0, [], {}, {}, {}],
	[0x0240C9B0, 0x80B18280, 0x80B18E80, [], {}, {}, {}],
	[0x0240D6F0, 0x80B18FC0, 0x80B197C0, [], {}, {}, {}],
	[0x0240DFB0, 0x80B19880, 0x80B19C00, [], {}, {}, {}],
	[0x0240E440, 0x80B19D10, 0x80B1A470, [], {}, {}, {}],
	[0x0240F370, 0x80B1AC40, 0x80B1B310, [], {}, {}, {}],
	[0x024134A0, 0x80B1ED70, 0x80B1EE00, [], {}, {}, {}],
	[0x02413590, 0x80B1EE60, 0x80B21D00, [], {}, {}, {}],
	[0x02416DF0, 0x80B226C0, 0x80B22CB0, [], {}, {}, {}],
	[0x024174B0, 0x80B22D80, 0x80B23230, [], {}, {}, {}],
	[0x02417A30, 0x80B23300, 0x80B23900, [], {}, {}, {}],
	[0x024180D0, 0x80B239A0, 0x80B250B0, [], {}, {}, {}],
	[0x02419A70, 0x80B25340, 0x80B25530, [], {}, {}, {}],
	[0x02419CC0, 0x80B25590, 0x80B276E0, [], {}, {}, {}],
	[0x0241C2B0, 0x80B27B80, 0x80B289C0, [], {}, {}, {}],
	[0x02420360, 0x80B2BC30, 0x80B2C100, [], {}, {}, {}],
	[0x024210B0, 0x80B2C980, 0x80B2D0C0, [], {}, {}, {}],
	[0x024218E0, 0x80B2D1B0, 0x80B2D770, [], {}, {}, {}],
	[0x02421F50, 0x80B2D820, 0x80B2DBB0, [], {}, {}, {}],
	[0x02422370, 0x80B2DC40, 0x80B2E490, [], {}, {}, {}],
	[0x02422CA0, 0x80B2E570, 0x80B2E8A0, [], {}, {}, {}],
	[0x02423030, 0x80B2E900, 0x80B2EE30, [], {}, {}, {}],
	[0x024235E0, 0x80B2EEB0, 0x80B2F330, [], {}, {}, {}],
	[0x02423AD0, 0x80B2F3A0, 0x80B2F7C0, [], {}, {}, {}],
	[0x02423F50, 0x80B2F820, 0x80B2FD10, [], {}, {}, {}],
	[0x024244B0, 0x80B2FD80, 0x80B30180, [], {}, {}, {}],
	[0x02424960, 0x80B30230, 0x80B30410, [], {}, {}, {}],
	[0x02424BC0, 0x80B30490, 0x80B306A0, [], {}, {}, {}],
	[0x02424E50, 0x80B30720, 0x80B30D30, [], {}, {}, {}],
	[0x02425520, 0x80B30DF0, 0x80B31300, [], {}, {}, {}],
	[0x02425AB0, 0x80B31380, 0x80B31920, [], {}, {}, {}],
	[0x024260F0, 0x80B319C0, 0x80B31CE0, [], {}, {}, {}],
	[0x02426490, 0x80B31D60, 0x80B32380, [], {}, {}, {}],
	[0x02426B60, 0x80B32430, 0x80B326F0, [], {}, {}, {}],
	[0x02426E90, 0x80B32760, 0x80B32BE0, [], {}, {}, {}],
	[0x024273A0, 0x80B32C70, 0x80B32F40, [], {}, {}, {}],
	[0x02427730, 0x80B33000, 0x80B33440, [], {}, {}, {}],
	[0x02427C80, 0x80B33550, 0x80B33F00, [], {}, {}, {}],
	[0x02428C00, 0x80B344D0, 0x80B348A0, [], {}, {}, {}],
	[0x02429030, 0x80B34900, 0x80B34A70, [], {}, {}, {}],
	[0x024291E0, 0x80B34AB0, 0x80B35910, [], {}, {}, {}],
	[0x0242A270, 0x80B35B40, 0x80B35F10, [], {}, {}, {}],
	[0x0242A6B0, 0x80B35F80, 0x80B366F0, [], {}, {}, {}],
	[0x0242AEC0, 0x80B36790, 0x80B36DF0, [], {}, {}, {}],
	[0x0242B5C0, 0x80B36E90, 0x80B37530, [], {}, {}, {}],
	[0x0242BD00, 0x80B375D0, 0x80B37930, [], {}, {}, {}],
	[0x0242C0C0, 0x80B37990, 0x80B37DD0, [], {}, {}, {}],
	[0x0242C570, 0x80B37E40, 0x80B382A0, [], {}, {}, {}],
	[0x0242CA50, 0x80B38320, 0x80B38840, [], {}, {}, {}],
	[0x0242CFE0, 0x80B388B0, 0x80B38CD0, [], {}, {}, {}],
	[0x0242D460, 0x80B38D30, 0x80B38E30, [], {}, {}, {}],
	[0x0242D5A0, 0x80B38E70, 0x80B39590, [], {}, {}, {}],
	[0x0242F140, 0x80B3AA10, 0x80B3C200, [], {}, {}, {}],
	[0x02430B40, 0x80B3C410, 0x80B3C6B0, [], {}, {}, {}],
	[0x02430E60, 0x80B3C730, 0x80B3CC70, [], {}, {}, {}],
	[0x024314F0, 0x80B3CDC0, 0x80B3D430, [], {}, {}, {}],
	[0x02431CE0, 0x80B3D5B0, 0x80B3D920, [], {}, {}, {}],
	[0x024320F0, 0x80B3D9C0, 0x80B3DDA0, [], {}, {}, {}],
	[0x024325B0, 0x80B3DE80, 0x80B3F680, [], {}, {}, {}],
	[0x02433FE0, 0x80B3F8B0, 0x80B3FDE0, [], {}, {}, {}],
	[0x02434600, 0x80B3FED0, 0x80B40010, [], {}, {}, {}],
	[0x024347A0, 0x80B40070, 0x80B40DF0, [], {}, {}, {}],
	[0x024356B0, 0x80B40F80, 0x80B41070, [], {}, {}, {}],
	[0x02435840, 0x80B41110, 0x80B45D90, [], {}, {}, {}],
	[0x0243ABF0, 0x80B464C0, 0x80B468D0, [], {}, {}, {}],
	[0x0243C360, 0x80B47C30, 0x80B48040, [], {}, {}, {}],
	[0x0243DAB0, 0x80B49380, 0x80B49D00, [], {}, {}, {}],
	[0x0243E580, 0x80B49E50, 0x80B4AD50, [], {}, {}, {}],
	[0x0243F730, 0x80B4B000, 0x80B4B430, [], {}, {}, {}],
	[0x0243FC30, 0x80B4B500, 0x80B4B8B0, [], {}, {}, {}],
	[0x024400F0, 0x80B4B9C0, 0x80B4C100, [], {}, {}, {}],
	[0x02440950, 0x80B4C220, 0x80B4C780, [], {}, {}, {}],
	[0x02440F90, 0x80B4C860, 0x80B4CE30, [], {}, {}, {}],
	[0x02441650, 0x80B4CF20, 0x80B4DA60, [], {}, {}, {}],
	[0x02442370, 0x80B4DC40, 0x80B4EFC0, [], {}, {}, {}],
	[0x02443880, 0x80B4F150, 0x80B51B30, [], {}, {}, {}],
	[0x02446680, 0x80B51F50, 0x80B527E0, [], {}, {}, {}],
	[0x02447020, 0x80B528F0, 0x80B52AC0, [], {}, {}, {}],
	[0x02447240, 0x80B52B10, 0x80B52FF0, [], {}, {}, {}],
	[0x02447820, 0x80B530F0, 0x80B54100, [], {}, {}, {}],
	[0x02448B00, 0x80B543D0, 0x80B54920, [], {}, {}, {}],
	[0x024491A0, 0x80B54A70, 0x80B550C0, [], {}, {}, {}],
	[0x02449880, 0x80B55150, 0x80B55840, [], {}, {}, {}],
	[0x0244A050, 0x80B55920, 0x80B55EB0, [], {}, {}, {}],
	[0x0244A720, 0x80B55FF0, 0x80B56780, [], {}, {}, {}],
	[0x0244B040, 0x80B56910, 0x80B5BC60, [], {}, {}, {}],
	[0x02451060, 0x80B5C930, 0x80B605F0, [], {}, {}, {}],
	[0x02455370, 0x80B60C40, 0x80B622D0, [], {}, {}, {}],
	[0x02456C60, 0x80B62530, 0x80B659C0, [], {}, {}, {}],
	[0x0245A910, 0x80B661E0, 0x80B66830, [], {}, {}, {}],
	[0x0245B130, 0x80B66A00, 0x80B66EC0, [], {}, {}, {}],
	[0x0245B670, 0x80B66F40, 0x80B67680, [], {}, {}, {}],
	[0x0245BE30, 0x80B67700, 0x80B67E20, [], {}, {}, {}],
	[0x0245C690, 0x80B67F60, 0x80B68050, [], {}, {}, {}],
	[0x0245C800, 0x80B680D0, 0x80B68D10, [], {}, {}, {}],
	[0x0245D8F0, 0x80B691D0, 0x80B6A6E0, [], {}, {}, {}],
	[0x0245F060, 0x80B6A940, 0x80B6BDC0, [], {}, {}, {}],
	[0x02460790, 0x80B6C070, 0x80B6DAD0, [], {}, {}, {}],
	[0x02462630, 0x80B6DF10, 0x80B6F090, [], {}, {}, {}],
	[0x02463B00, 0x80B6F3E0, 0x80B6FA80, [], {}, {}, {}],
	[0x02464300, 0x80B6FBE0, 0x80B70D00, [], {}, {}, {}],
	[0x024656C0, 0x80B70FA0, 0x80B71810, [], {}, {}, {}],
	[0x02466040, 0x80B71920, 0x80B71D60, [], {}, {}, {}],
	[0x02466530, 0x80B71E10, 0x80B72AB0, [], {}, {}, {}],
	[0x024672F0, 0x80B72BD0, 0x80B74000, [], {}, {}, {}],
	[0x02468990, 0x80B74270, 0x80B75540, [], {}, {}, {}],
	[0x02469F00, 0x80B757E0, 0x80B75DD0, [], {}, {}, {}],
	[0x0246A630, 0x80B75F10, 0x80B76050, [], {}, {}, {}],
	[0x0246A7D0, 0x80B760B0, 0x80B76540, [], {}, {}, {}],
	[0x0246AD20, 0x80B76600, 0x80B772C0, [], {}, {}, {}],
	[0x0246BC90, 0x80B77570, 0x80B78850, [], {}, {}, {}],
	[0x0246D0F0, 0x80B789D0, 0x80B790E0, [], {}, {}, {}],
	[0x0246D920, 0x80B79200, 0x80B795E0, [], {}, {}, {}],
	[0x0246DD90, 0x80B79670, 0x80B79CC0, [], {}, {}, {}],
	[0x0246E4B0, 0x80B79D90, 0x80B7ACF0, [], {}, {}, {}],
	[0x0246F590, 0x80B7AE70, 0x80B7B260, [], {}, {}, {}],
	[0x02470570, 0x80B7BE50, 0x80B7BF80, [], {}, {}, {}],
	[0x02470720, 0x80B7C000, 0x80B7C0E0, [], {}, {}, {}],
	[0x02470870, 0x80B7C150, 0x80B7E2B0, [], {}, {}, {}],
	[0x02472E20, 0x80B7E700, 0x80B7EB60, [], {}, {}, {}],
	[0x024732E0, 0x80B7EBC0, 0x80B7ECC0, [], {}, {}, {}],
	[0x02473430, 0x80B7ED10, 0x80B7F720, [], {}, {}, {}],
	[0x02473F80, 0x80B7F860, 0x80B80340, [], {}, {}, {}],
	[0x02474BC0, 0x80B804A0, 0x80B80C10, [], {}, {}, {}],
	[0x02475410, 0x80B80CF0, 0x80B83C90, [], {}, {}, {}],
	[0x02479E40, 0x80B85720, 0x80B86320, [], {}, {}, {}],
	[0x0247AC00, 0x80B864E0, 0x80B866F0, [], {}, {}, {}],
	[0x0247AE70, 0x80B86750, 0x80B87130, [], {}, {}, {}],
]

dcall = [
]

cache = [
]