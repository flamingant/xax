#ifdef __cplusplus /*Z*/
extern "C" {
#endif

#ifndef __stimer_h
#define __stimer_h

typedef struct __struct_TE STIMER ;

typedef u32 (*STF)(STIMER *,int,u32) ;

struct __struct_TE {
    STF		f ;
    void	*c ;
    int		ms_start ;
    int		ms_curr ;
    int		state ;
    } ;

#define STM_EXPIRE	0

#define STS_KILLED	1
#define STS_REARM	2
#define STS_PAUSE	4
#define STS_RETRIGGER	8

extern STIMER *stimer_alloc(void) ;

extern STIMER *stimer_build(STF f,void *c) ;

extern STIMER *stimer_add(STF f,void *c,int delay) ;
extern STIMER *stimer_rearm(STIMER *) ;

extern void stimer_kill(STIMER *te) ;

extern int stimer_clock_started(void) ;

extern void stimer_clock_start(void) ;
extern void stimer_clock_stop(void) ;
extern void stimer_clock_restart(void) ;

extern void stimer_block(void) ;
extern void stimer_unblock(void) ;

extern void stimer_init(void) ;
extern void stimer_onexit(void) ;

extern void stimer_pause(STIMER *st) ;
extern void stimer_unpause(STIMER *st) ;

extern void stimer_retrigger(STIMER *) ;
extern void stimer_expire_now(STIMER *) ;

#endif

#ifdef __cplusplus /*Z*/
}
#endif
