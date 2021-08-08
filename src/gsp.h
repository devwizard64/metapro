#ifndef __GSP_H__
#define __GSP_H__

#include "types.h"

#ifndef __ASSEMBLER__

extern void gsp_init(void);
extern void gsp_destroy(void);
extern void gsp_cache(void);
extern void gsp_update(void *, u32 *);

#endif /* __ASSEMBLER__ */

#endif /* __GSP_H__ */
