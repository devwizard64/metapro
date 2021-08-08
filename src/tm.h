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

extern void tm_cpu_start(void);
extern void tm_gsp_start(void);
extern void tm_asp_start(void);
extern void tm_cpu_end(enum tm_index);
extern void tm_gsp_end(enum tm_index);
extern void tm_asp_end(enum tm_index);
extern void tm_update(void);
extern void tm_draw(void);

#endif /* __ASSEMBLER__ */

#endif /* __TIM_H__ */
