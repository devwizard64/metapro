#include "types.h"
#include "app.h"
#include "cpu.h"
#include "sys.h"
#include "mtx.h"

void lib_guPerspective(void)
{
    f32 mf[4][4];
    float y = ARG_F(a2);
#ifdef VIDEO_DYNRES
    float a = video_aspect;
#else
    float a = ARG_F(a3);
#endif
    float n = *__f32(sp+0x10);
    float f = *__f32(sp+0x14);
    mtx_perspective(mf, y, a, n, f);
    mtx_write(__dram(a0), &mf[0][0]);
}
