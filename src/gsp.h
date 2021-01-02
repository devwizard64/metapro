#ifndef _GSP_H_
#define _GSP_H_

#include "types.h"

#ifndef __ASSEMBLER__

extern void gsp_init(void);
extern void gsp_destroy(void);
extern void gsp_cache(void);
extern void gsp_update(void *, u32 *);

#endif /* __ASSEMBLER__ */

#endif /* _GSP_H_ */
