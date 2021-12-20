#ifndef __MTX_H__
#define __MTX_H__

#include "types.h"

#ifndef __ASSEMBLER__

#define MDOT3(m, i) ((m)[0][i]*x + (m)[1][i]*y + (m)[2][i]*z)
#define IDOT3(m, i) ((m)[i][0]*x + (m)[i][1]*y + (m)[i][2]*z)
#define MDOT4(m, i) (MDOT3(m, i) + (m)[3][i])

#define mtx_ortho_bg(mf, l, r, b, t, n, f)                          \
{                                                                   \
    float _y = (1.0F/2) * ((t)+(b));                                \
    float _h = (1.0F/2) * ((t)-(b)) / ((3.0F/4.0F)*video_aspect);   \
    mtx_ortho(mf, l, r, _y-_h, _y+_h, n, f);                        \
}

#define mtx_ortho_fg(mf, l, r, b, t, n, f)                          \
{                                                                   \
    float _x = (1.0F/2) * ((r)+(l));                                \
    float _w = (1.0F/2) * ((r)-(l)) * ((3.0F/4.0F)*video_aspect);   \
    mtx_ortho(mf, _x-_w, _x+_w, b, t, n, f);                        \
}

extern void mtx_read(f32 *dst, const s16 *src);
extern void mtx_write(s16 *dst, const f32 *src);
extern void mtx_cat(f32 mf[4][4], f32 a[4][4], f32 b[4][4]);
extern void mtx_identity(f32 mf[4][4]);
extern void mtx_ortho(
    f32 mf[4][4], float l, float r, float b, float t, float n, float f
);
extern void mtx_perspective(
    f32 mf[4][4], float fovy, float aspect, float n, float f
);

#endif /* __ASSEMBLER__ */

#endif /* __MTX_H__ */
