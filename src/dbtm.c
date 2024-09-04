#include "types.h"
#include "dbtm.h"

#ifdef DEBUG

DBTM dbtm_data[DBTM_MAX];
DBTM dbtm_cpu[8];
DBTM dbtm_gsp[8];
DBTM dbtm_asp[8];
u8 dbtm_cpu_i;
u8 dbtm_gsp_i;
u8 dbtm_asp_i;

void dbtm_update(void)
{
	memset(dbtm_data, 0, sizeof(dbtm_data));
#ifdef __NDS__
	cpuStartTiming(0);
#endif
}

void dbtm_draw(void)
{
	static const char str[DBTM_MAX][12] =
	{
		"CPU THREAD4",
		"CPU THREAD5",
		"RSP AUDTASK",
		"RSP GFXTASK",
	};
	int i;
	pdebug("********************************\n");
	for (i = 0; i < DBTM_MAX; i++)
	{
		u32 usec = 1000000 * dbtm_data[i]/DBTM_CLOCK;
		pdebug("%s:%8" FMT_u "u\n", str[i], usec);
	}
}

#endif
