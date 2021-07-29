#ifndef __PM_H__
#define __PM_H__

#include "types.h"

#ifndef __ASSEMBLER__

enum pm_index
{
    PM_CPU_THREAD4,
    PM_CPU_THREAD5,
    PM_RSP_AUDTASK,
    PM_RSP_GFXTASK,
    PM_LEN,
};

extern void pm_cpu_start(void);
extern void pm_gsp_start(void);
extern void pm_asp_start(void);
extern void pm_cpu_end(enum pm_index);
extern void pm_gsp_end(enum pm_index);
extern void pm_asp_end(enum pm_index);
extern void pm_update(void);
extern void pm_draw(void);

#endif /* __ASSEMBLER__ */

#endif /* _PM_H_ */
