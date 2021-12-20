#include "types.h"
#include "app.h"
#include "cpu.h"
#include "sys.h"
#include "mtx.h"

void lib_guOrtho(void)
{
    f32 mf[4][4];
    float l = ARG_F(a1);
    float r = ARG_F(a2);
    float b = ARG_F(a3);
    float t = *__f32(sp+0x10);
    float n = *__f32(sp+0x14);
    float f = *__f32(sp+0x18);
#ifdef VIDEO_DYNRES
    if (n == 0 && f == 3)
    {
    #ifdef __NDS__
        n = -3;
        f = 0;
    #endif
        mtx_ortho_bg(mf, l, r, b, t, n, f);
    }
    else
    {
        mtx_ortho_fg(mf, l, r, b, t, n, f);
    }
#else
    mtx_ortho(mf, l, r, b, t, n, f);
#endif
    mtx_write(__dram(a0), &mf[0][0]);
}
