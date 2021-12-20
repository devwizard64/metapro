#ifndef __GSP_H__
#define __GSP_H__

#include "types.h"

#ifndef __ASSEMBLER__

extern void gsp_init(void);
extern void gsp_exit(void);
extern void gsp_cache(void);
extern void gsp_update(PTR ucode, u32 *dl);
extern void gsp_image(void *img);

#endif /* __ASSEMBLER__ */

#endif /* __GSP_H__ */
