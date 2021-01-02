#ifndef _PM_H_
#define _PM_H_

#include "types.h"

#ifndef __ASSEMBLER__

enum pm_index_t
{
    PM_CPU_THREAD4,
    PM_CPU_THREAD5,
    PM_RSP_AUDTASK,
    PM_RSP_GFXTASK,
    PM_LEN,
};

extern void pm_cpu_start(void);
extern void pm_rsp_start(void);
extern void pm_cpu_end(enum pm_index_t);
extern void pm_rsp_end(enum pm_index_t);
extern void pm_update(void);
extern void pm_draw(void);

#endif /* __ASSEMBLER__ */

#endif /* _PM_H_ */
