#include <errno.h>

#include "types.h"
#include "lib.h"
#include "app.h"

#ifdef APP_SEQ

static char *audio;
static const u32 audioseq[4] = {0x00030001, 0x10, 0x10000};
static u8 audiobnk[0x100];
static u32 ctl = _AudioctlSegmentRomStart;
static u32 tbl = _AudiotblSegmentRomStart;
static const char *seq;

void seq_init(int argc, char *argv[])
{
	static const char cmd[4][4] = {"bin", "map", "ctl", "tbl"};
	char str[4096];
	char *arg[4] = {0};
	const char *path = PATH_APP;
	int i;
	FILE *fp;
	size_t size;
	if (argc < 3)
	{
		fprintf(stderr, "usage: %s <seq> <bnk> [...]\n", argv[0]);
		exit(1);
	}
	if (!(fp = fopen("config.txt", "r")))
	{
		eprint("could not read '%s'\n", "config.txt");
	}
	while (fgets(str, sizeof(str), fp))
	{
		char *s;
		if (!(s = strtok(str, "\t "))) continue;
		for (i = 0; i < 4; i++)
		{
			if (!strcmp(s, cmd[i]))
			{
				if (!(s = strtok(NULL, "\n\r"))) break;
				if (arg[i]) free(arg[i]);
				arg[i] = strdup(s);
				break;
			}
		}
	}
	fclose(fp);
	if (arg[0]) path = arg[0];
	if (arg[1])
	{
		if (!(fp = fopen(arg[1], "r")))
		{
			eprint("could not read '%s'\n", arg[1]);
		}
		while (fgets(str, sizeof(str), fp))
		{
			char *s;
			u32 addr;
			if (!(s = strtok(str, "\t "))) continue;
			errno = 0;
			addr = strtol(s, NULL, 0);
			if (errno) continue;
			if (!(s = strtok(NULL, "\t\n\r "))) continue;
			if      (arg[2] && !strcmp(arg[2], s)) ctl = addr;
			else if (arg[3] && !strcmp(arg[3], s)) tbl = addr;
		}
		fclose(fp);
	}
	else
	{
		if (arg[2])
		{
			errno = 0;
			ctl = strtol(arg[2], NULL, 0);
			if (errno) eprint("invalid ctl: %s\n", arg[2]);
		}
		if (arg[3])
		{
			errno = 0;
			tbl = strtol(arg[3], NULL, 0);
			if (errno) eprint("invalid tbl: %s\n", arg[3]);
		}
	}
	if (!(fp = fopen(path, "rb")))
	{
		eprint("could not read '%s'\n", path);
	}
	fseek(fp, 0, SEEK_END);
	audio = malloc(size = ftell(fp));
	fseek(fp, 0, SEEK_SET);
	fread(audio, 1, size, fp);
	fclose(fp);
	__byteswap(audio, audio, size);
	for (i = 0; i < 4; i++) if (arg[i]) free(arg[i]);
	seq = argv[1];
	if (argc > 2+253) eprint("too many banks\n");
	audiobnk[0] = 0;
	audiobnk[1] = 2;
	audiobnk[2] = argc-2;
	for (i = 2; i < argc; i++)
	{
		errno = 0;
		audiobnk[3+i-2] = strtol(argv[i], NULL, 0);
		if (errno) eprint("invalid bank: %s\n", argv[i]);
	}
}

/*
0x80226B4C = seq
0x80226B50 = ctl
0x80226B54 = tbl
0x80226B58 = bnk
*/

void lib_80248AF0(void)
{
	Na_Output(0);
	Na_Mode(0);
	Na_BgmPlay(0, 0, 0);
	for (;;)
	{
		Na_update();
		osRecvMesg(0x8033B010, NULLPTR, OS_MESG_BLOCK);
	}
}

void lib_osPiStartDma_seq(void)
{
	u32   devAddr   = a3;
	void *vAddr     = cpu_ptr(*cpu_s32(sp+0x10));
	u32   nbytes    =        (*cpu_u32(sp+0x14));
	OSMesgQueue *mq = cpu_ptr(*cpu_s32(sp+0x18));
	if (devAddr >= 0x80000000)
	{
		memcpy(vAddr, audio+tbl + (devAddr-0x80000000), nbytes);
	}
	else if (devAddr >= 0x800000)
	{
		memcpy(vAddr, audio+ctl + (devAddr-0x800000), nbytes);
	}
	else if (devAddr == _AudioseqSegmentRomStart)
	{
		memcpy(vAddr, audioseq, nbytes);
	}
	else if (devAddr == _AudioseqSegmentRomStart+0x10)
	{
		FILE *fp;
		if ((fp = fopen(seq, "rb")))
		{
			size_t size;
			fseek(fp, 0, SEEK_END);
			size = ftell(fp);
			fseek(fp, 0, SEEK_SET);
			fread(vAddr, 1, size, fp);
			fclose(fp);
			__byteswap(vAddr, vAddr, size);
		}
		else
		{
			eprint("could not read '%s'\n", seq);
		}
	}
	else if (devAddr == _AudiobnkSegmentRomStart)
	{
		__byteswap(vAddr, audiobnk, nbytes);
	}
	else
	{
		eprint("DMA 0x%08X 0x%08X %u\n", *cpu_u32(sp+0x10), devAddr, nbytes);
	}
	v0 = mesg_send(mq, 0, OS_MESG_NOBLOCK);
}

#endif
