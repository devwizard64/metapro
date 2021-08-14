#ifndef __TM_H__
#define __TM_H__

#include "types.h"

#ifndef __ASSEMBLER__

enum tm_index
{
    TM_CPU_THREAD4,
    TM_CPU_THREAD5,
    TM_RSP_AUDTASK,
    TM_RSP_GFXTASK,
    TM_LEN,
};

#ifdef __DEBUG__
extern void tm_cpu_start(void);
extern void tm_gsp_start(void);
extern void tm_asp_start(void);
extern void tm_cpu_end(enum tm_index index);
extern void tm_gsp_end(enum tm_index index);
extern void tm_asp_end(enum tm_index index);
extern void tm_update(void);
extern void tm_draw(void);
#else
#define tm_cpu_start()
#define tm_gsp_start()
#define tm_asp_start()
#define tm_cpu_end(index)
#define tm_gsp_end(index)
#define tm_asp_end(index)
#define tm_update()
#define tm_draw()
#endif

#endif /* __ASSEMBLER__ */

#endif /* __TM_H__ */
