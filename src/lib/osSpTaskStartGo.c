#include "types.h"
#include "cpu.h"
#include "rsp.h"
#include "sys.h"

#include "ultra64.h"

void lib_osSpTaskStartGo(void)
{
    OSTask task;
    memcpy(&task, cpu_ptr(a0), sizeof(OSTask));
    task.ucode_boot         = __tlb(task.ucode_boot);
    task.ucode              = __tlb(task.ucode);
    task.ucode_data         = __tlb(task.ucode_data);
    task.dram_stack         = __tlb(task.dram_stack);
    task.output_buff        = __tlb(task.output_buff);
    task.output_buff_size   = __tlb(task.output_buff_size);
    task.data_ptr           = __tlb(task.data_ptr);
    task.yield_data_ptr     = __tlb(task.yield_data_ptr);
    switch (task.type)
    {
        case M_GFXTASK:
            rsp_gfxtask(task.ucode, &cpu_dram[task.data_ptr]);
            os_event(&os_event_table[OS_EVENT_DP]);
            break;
    #ifdef __LLE__
        default:
            rsp_main(&task);
            break;
    #else
        case M_AUDTASK:
        #ifndef __NDS__
        #if defined(APP_UNSM) /* && defined(APP_E0) */
            rsp_audtask(&cpu_dram[task.data_ptr], task.data_size);
        #endif
        #endif
            break;
    #ifdef APP_UCZL
        case M_NJPEGTASK:
            if (task.ucode == 0x00006210) break;
            /* 0x000E6BC0 = njpgdspMain */
            break;
    #endif
        default:
            wdebug("unknown task %d\n", task.type);
            break;
    #endif
    }
    os_event(&os_event_table[OS_EVENT_SP]);
}
