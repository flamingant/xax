#ifdef __cplusplus /*Z*/
extern "C" {
#endif

#include	"arg.h"
#include	"dp.h"

#include	"npu_util.h"

#include	<unistd.h>
#include	<stdio.h>
#include	<sys/socket.h>
#include	<arpa/inet.h>
#include	<stdlib.h>
#include	<netdb.h>
#include	<string.h>

#include	<fcntl.h>
#include	<errno.h>

#include	<sys/select.h>
#include	<sys/param.h>

#include	<signal.h>
#include	<time.h>

#include	"pu/mt.h"

#include	"pu/rcons.h"

#include	"stimer.h"
/* ================================================================ */
typedef struct timeval TIME ;

struct clocki {    
    TIME	time[1] ;
    int		ticks_old ;
    int		ticks_new ;
    int		ms_diff ;
    int		running ;

    timer_t	tid ;
    struct sigevent ev[1] ;
    } ;

static struct clocki clocki ;

static void SIGALRM_h(int sig, siginfo_t *si, void *uc) ;

static void stimer_check(void) ;

#define MSTICK	1000

#define SIGBAG	(SIGRTMAX-1)
#define SIG	SIGBAG

extern void stimer_block(void)
{
    int		e ;
    sigset_t mask;  
    sigemptyset(&mask);
    sigaddset(&mask, SIG);
    if ((e = sigprocmask(SIG_SETMASK, &mask, NULL) == -1))
	errorshow("sigprocmask fail\n") ;
    }

extern void stimer_unblock(void)
{
    int		e ;
    sigset_t mask;  
    sigemptyset(&mask);
    sigaddset(&mask, SIG);
    if ((e = sigprocmask(SIG_UNBLOCK, &mask, NULL) == -1))
	errorshow("sigprocmask fail\n") ;
    }

extern void stimer_clock_restart(void)
{
    static struct itimerspec it,ct ;
    it.it_value.tv_sec = MSTICK / 1000  ;
    it.it_value.tv_nsec = (1000000 * MSTICK) % 1000000000 ;
    it.it_interval.tv_sec = it.it_value.tv_sec ;
    it.it_interval.tv_nsec = (1000000 * MSTICK) % 1000000000 ;
    timer_settime(clocki.tid,0,&it,&ct) ;
    }

extern void stimer_clock_stop(void)
{
    static struct itimerspec it,ct ;
    if (!clocki.running) return ;
    clocki.running = 0 ;
    it.it_interval.tv_sec = 0 ;
    it.it_interval.tv_nsec = 0 ;
    it.it_value.tv_sec = 0 ;
    it.it_value.tv_nsec = 0 ;
    timer_settime(clocki.tid,0,&it,&ct) ;
    }

extern void stimer_clock_start(void)
{
    stimer_init() ;
    stimer_clock_restart() ;
    clocki.running = 1 ;
    }

extern int stimer_clock_started(void)
{
    return(clocki.running == 1) ;
    }

static void SIGALRM_h(int sig, siginfo_t *si, void *uc)
{
    /* this looks stupid but allows alternate "elapsed time" strategy transparently */
    clocki.ticks_old = clocki.ticks_new ;
    clocki.ticks_new++ ;
    clocki.ms_diff = MSTICK * (clocki.ticks_new - clocki.ticks_old) ;
    stimer_check() ;
    }

/* ================================================================ */
#include	"stimer.h"
#include	<pu/rat.h>

static struct {
    RCONS	*r ;
    RAY		ra[1] ;
    } tt ;

static void stimer_ra_init(void)
{
    tt.ra->rat->unit_size = sizeof(STIMER) ;
    tt.ra->rat->units_per_node = 8 ;
    ray_init(tt.ra) ;
    }

static void stimer_free(STIMER *st)
{
    ray_free(tt.ra,st) ;
    }

static STIMER *stimer_alloc_unsafe(void)
{
    if (RAY_EMPTY(tt.ra)) stimer_ra_init() ;
    return (STIMER *) ray_ualloc(tt.ra) ;
    }

extern STIMER *stimer_alloc(void)
{
    STIMER *s = stimer_alloc_unsafe() ;
    s->f = 0 ;
    s->c = 0 ;
    s->state = 0 ;
    return s ;
}

extern void stimer_queue(STIMER *st)
{
    tt.r = rcons_cons(st,tt.r) ;
    }

extern void stimer_unpause(STIMER *st)
{
    st->state &= ~STS_PAUSE ;
    }

extern void stimer_pause(STIMER *st)
{
    st->state |= STS_PAUSE ;
    }

extern void stimer_reset(STIMER *st)
{
    st->ms_curr = st->ms_start ;
    }

extern void stimer_set(STIMER *st,int delay)
{
    st->ms_start = delay ;
    st->ms_curr = st->ms_start ;
    }

extern STIMER *stimer_build(STF f,void *c)
{
    STIMER *st = stimer_alloc_unsafe() ;
    st->f = f ;
    st->c = c ;
    st->state = 0 ;
    return(st) ;
    }

extern STIMER *stimer_add(STF f,void *c,int delay)
{
    STIMER *st = stimer_build(f,c) ;
    stimer_set(st,delay) ;
    stimer_queue(st) ;
    return(st) ;
    }

extern void stimer_retrigger(STIMER *st)
{
    st->state |= STS_RETRIGGER ; 
}

extern STIMER *stimer_rearm(STIMER *st)
{
    return(stimer_add(st->f,st->c,st->ms_start)) ;
    }
    
extern void stimer_expire_now(STIMER *st)
{
    st->ms_curr = 0 ;
    }

static u32 stf_null(STIMER *st,int m,u32 a)
{
    return(0) ;
    }
    
extern void stimer_kill(STIMER *st)
{
    if (!ANZW(st)) {return ;}
    if (clocki.tid) {
	st->state |= STS_KILLED ;
	st->state &= ~(STS_REARM | STS_PAUSE) ;
	st->f = stf_null ;
	st->ms_curr = 0 ;
	}
    else {
	while (rcons_delete(st,&tt.r)) ;
	stimer_free(st) ;
	}
    }

static void stimer_check()
{
    RCONS	*rs = 0 ;
    RCONS	*rh = tt.r ;
    RCONS	*r,**rr = &rh ;
    stimer_block() ;
    tt.r = 0 ;
#if defined(STIMER_DEBUG) && STIMER_DEBUG > 1
    if (!(clocki.ticks_new % 100)) printf("tick %d\n",clocki.ticks_new) ;
#endif
    while (r = *rr) {
	STIMER	*st = (STIMER *) r->car ;
	if (!(st->state & STS_PAUSE)) st->ms_curr -= clocki.ms_diff ;
	if (st->ms_curr <= 0) {
	    do {
		st->state &= ~STS_RETRIGGER ;
		st->f(st,STM_EXPIRE,0) ;
		} while (st->state & STS_RETRIGGER) ;
	    *rr = r->cdr ;
	    rcons_free(r) ;
	    if (st->state & STS_REARM) {
		stimer_reset(st) ;
		rs = rcons_cons(st,rs) ;
		}
	    else 
		stimer_free(st) ;
	}
	else {
	    rr = &r->cdr ;
	    }
	}
    *rcons_tail(&rh) = rs ;
    *rcons_tail(&tt.r) = rh ;
    stimer_unblock() ;
    }

/* ================================================================ */
extern void stimer_onexit(void)
{
    tt.r = rcons_free_list(tt.r) ;
    if (!RAY_EMPTY(tt.ra)) ray_destroy(tt.ra) ;
    if (clocki.tid) {
	timer_delete(clocki.tid) ;
	clocki.tid = 0 ;
	}
}

extern void stimer_init(void)
{
    if (clocki.tid) return ;
{
    int e ;
    struct sigaction sa;
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = SIGALRM_h ;
    sigemptyset(&sa.sa_mask);
    if ((e = sigaction(SIG, &sa, NULL) == -1))
	errorshow("sigaction fail\n") ;

    stimer_block() ;

    clocki.ev->sigev_notify = SIGEV_SIGNAL ;
    clocki.ev->sigev_signo = SIG ;
    clocki.ev->sigev_value.sival_int = 0x3333 ;
    clocki.ev->sigev_notify_function = 0 ;
    clocki.ev->sigev_notify_attributes = 0 ;

    if (timer_create(CLOCK_REALTIME,clocki.ev,&clocki.tid) == -1)
	errorfatal("clock not created\n") ;

    stimer_unblock() ;
    }
    }


#ifdef __cplusplus /*Z*/
}
#endif
