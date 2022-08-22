#ifndef __DBTM_H__
#define __DBTM_H__

#define DBTM_CPU_THREAD4        0
#define DBTM_CPU_THREAD5        1
#define DBTM_RSP_AUDTASK        2
#define DBTM_RSP_GFXTASK        3
#define DBTM_MAX                4

#ifdef DEBUG

#ifdef __NATIVE__
#define dbtm()          clock()
#define DBTM_CLOCK      CLOCKS_PER_SEC
typedef clock_t DBTM;
#endif
#ifdef GEKKO
#define dbtm()          gettime()
#define DBTM_CLOCK      (TB_BUS_CLOCK/4)
typedef u64 DBTM;
#endif
#ifdef __NDS__
#define dbtm()          cpuGetTiming()
#define DBTM_CLOCK      BUS_CLOCK
typedef u32 DBTM;
#endif
#ifdef __3DS__
#define dbtm()          svcGetSystemTick()
#define DBTM_CLOCK      SYSCLOCK_ARM11
typedef u64 DBTM;
#endif

extern DBTM dbtm_data[DBTM_MAX];
extern DBTM dbtm_cpu[];
extern DBTM dbtm_gsp[];
extern DBTM dbtm_asp[];
extern u8 dbtm_cpu_i;
extern u8 dbtm_gsp_i;
extern u8 dbtm_asp_i;

#define dbtm_cpu_start()    (dbtm_cpu[dbtm_cpu_i++] = dbtm())
#define dbtm_gsp_start()    (dbtm_gsp[dbtm_gsp_i++] = dbtm())
#define dbtm_asp_start()    (dbtm_asp[dbtm_asp_i++] = dbtm())
#define dbtm_cpu_end(i) (dbtm_data[i] += dbtm() - dbtm_cpu[--dbtm_cpu_i])
#define dbtm_gsp_end(i) (dbtm_data[i] += dbtm() - dbtm_gsp[--dbtm_gsp_i])
#define dbtm_asp_end(i) (dbtm_data[i] += dbtm() - dbtm_asp[--dbtm_asp_i])

extern void dbtm_update(void);
extern void dbtm_draw(void);

#else

#define dbtm_cpu_start()
#define dbtm_gsp_start()
#define dbtm_asp_start()
#define dbtm_cpu_end(i)
#define dbtm_gsp_end(i)
#define dbtm_asp_end(i)

#define dbtm_update()
#define dbtm_draw()

#endif

#endif /* __DBTM_H__ */
