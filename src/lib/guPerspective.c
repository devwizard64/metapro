#include "types.h"
#include "app.h"
#include "cpu.h"
#include "sys.h"
#include "mtx.h"

void lib_guPerspective(void)
{
	f32 mf[4][4];
	float fovy = ARG_F(a2);
#ifdef VIDEO_DYNRES
	float aspect = video_aspect;
#else
	float aspect = ARG_F(a3);
#endif
	float n = *cpu_f32(sp+0x10);
	float f = *cpu_f32(sp+0x14);
	*cpu_u16(a1) = 0xFFFF;
	mtx_perspective(mf, fovy, aspect, n, f);
	mtx_wr(cpu_ptr(a0), &mf[0][0]);
}
