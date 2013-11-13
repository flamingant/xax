#ifdef __cplusplus /*Z*/
extern "C" {
#endif

#include	"arg.h"
#include	"dp.h"

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

#include	"pu/mt.h"

#include	"pu/rcons.h"
#include	"pu/regex.h"

#include	"stimer.h"

#include	"log.h"
/* ~# use collect ; collect::register_all('logsec','^LOGSEC (\w+)','$1') ; #~ */

#include	"uf.h"
#include	"tod.h"
#include	"dht.h"

#include	"common.h"

#include	"tc_sql.h"

/* ================================================================ */
typedef struct __struct_EFD EFD ;

struct __struct_EFD {
    int		state ;
    int		fail ;
    int		fd ;
    } ;

/* ================================================================ */
#include	"dts.h"

typedef struct {
    int		state ;
    EFD		efd[1] ;
    MT		in[1] ;
    MT		parse[1] ;
    STIMER	*st ;
    UF		*uf ;
    int		tid_max ;
    DATETIMESTRING time ;
    int		catchup ;
    } IUC ;

#define IUCS_INIT	1

typedef struct {
    IUC		iuc_anno[1] ;
    REGEX	rx_torrent[1] ;
    REGEX	rx_timestamp[1] ;
    int		init ;
    } IUG ;

static IUG iug[1] ;

LOGSEC lc_irc_parse[] = {"IRC_PARSE",1} ;

static void tf_put(char *pb,int cb)
{
    log_write(lc_irc_parse,pb,cb) ;
}

static void tf_printf(char *fmt,...)
{
    va_list	va ;
    va_start(va,fmt) ;
    log_vprintf(lc_irc_parse,fmt,va) ;
    }

static int parse_tanno(TANNO *t,REGEX *r,char *s,char *e)
{
    int n = e-s ;
    int x = re_search(r,s,n,0,n) ;
    memset(t,0,sizeof(TANNO)) ;
    if (x == -1) return 0 ;
{
    t->artist = s + r->reg[1].start ;
    t->tags = s + r->reg[0].end ;

    s[r->reg[1].end] = 0 ;
    s[r->reg[2].end] = 0 ;
    s[r->reg[3].end] = 0 ;

    t->gid = strtol(s + r->reg[2].start,0,10) ;
    t->tid = strtol(s + r->reg[3].start,0,10) ;

    t->title = strstr(t->artist," - ") ;
    if (!t->title) return 1 ;
    *(t->title) = 0 ;		/* zero terminate artist string */
    t->title += 3 ;

    t->format = strstr(t->title," - ") ;
    if (!t->format) return 1 ;
    *(t->format) = 0 ;		/* zero terminate title string */
    t->format += 3 ;

    s = t->title ;

    while (*s && *++s != '[') ;
    if (!*s) return 1 ;
    s[-1] = 0 ;
    t->year = ++s ;
    while (*s && *++s != ']') ;
    *s++ = 0 ;
    
    while (*s && *++s != '[') ;
    if (!*s) return 1 ;
    s[-1] = 0 ;
    t->type = ++s ;
    while (*++s != ']') ;
    *s++ = 0 ;
    
    return(1) ;
}
}

/*     012345678901234567890123 */
/* Wed Jan 30 15:11:48 2013 */

static int parse_irc_session(IUC *iuc,char *s,char *e)
{
    REGEX *r = iug->rx_timestamp ;
    int n = e-s ;
    int x = re_search(r,s,n,0,n) ;
    if (x == -1) return 0 ;
{
    DATETIMESTRING *ts = &iuc->time ;
    char *tp = s + r->reg[2].start + 4 ;
    int m = dts_month_1(tp) ;
    
    dts_init(ts) ;
    ts->year[0]		= tp[16+0] ;
    ts->year[1]		= tp[16+1] ;
    ts->year[2]		= tp[16+2] ;
    ts->year[3]		= tp[16+3] ;
    ts->month[0]	= '0' + m / 10 ;
    ts->month[1]	= '0' + m % 10 ;
    ts->day[0]		= tp[4+0] ;
    ts->day[1]		= tp[4+1] ;

    dts_copy_hh_mm_ss(ts,tp+7) ;

    }
    return 1 ;
    }

static void tanno_dump(TANNO *t)
{
    tf_printf("artist = %s\n",t->artist) ;
/*
    tf_printf("title = %s\n",t->title) ;
    tf_printf("format = %s\n",t->format) ;
    tf_printf("tid = %s\n",t->tid) ;
    tf_printf("tags = %s\n",t->tags) ;
    tf_printf("\n") ;
*/
    }

/* ================================================================ */
/* we don't really want to backlog every single unloaded torrent
   this may only get called once a second, but if we allow 64k
   of lines to be read in, that will strain the network.

   We also cannot allow this process to block the other tasks.

   We may need a way of starting a new download whenever the previous
   one has finished.
   */

static void parse_line(IUC *iuc,char *s,char *e)
{
    TANNO	t[1] ;
    if (s == e || !*s) return ;
    if (parse_tanno(t,iug->rx_torrent,s,e)) {
	dts_copy_hh_mm_ss(&iuc->time,s+1) ;
	if (t->tid > iuc->tid_max) {
	    t->time_seen = iuc->time.year ;
	    tanno_add(t) ;
	    iuc->tid_max = t->tid ;
	    if (!iuc->catchup) {
		dht_add_torrent_url_i(t->tid) ;
		}
	    }
	return ;
	}
    if (parse_irc_session(iuc,s,e)) {
	return ;
	}
}

static int irc_log_drain(IUC *iuc)
{
    EFD *efd = iuc->efd ;
    MT	*mt = iuc->in ;
    int		n,pos ;
    pos = (int) lseek(efd->fd,0,SEEK_CUR) ;
    n = read(efd->fd, mt->c, mt->e - mt->c) ;
    if (n < 0) return n ;
    if (n == 0) return n ;
    mt->c += n ;
    while (iuc->parse->c < mt->c) {
	char	*p = iuc->parse->c ;
	while (*p != '\n') {
	    if (++p == mt->c) goto done ;
	    }
	*(p-1) = 0 ;
	parse_line(iuc,iuc->parse->c,p) ;
	iuc->parse->c = ++p ;
	}
done:
{
    int nr = mt->c - iuc->parse->c ;
    memcpy(mt->s,iuc->parse->c,nr) ;
    mt->c = mt->s + nr ;
    iuc->parse->c = mt->s ;
    return n ;
    }
    }

static void irc_wakeup(UF *uf)
{
    irc_log_drain((IUC *) uf->d.v) ;
    }

static u32 irc_uff(UF *uf,int m,u32 a)
{
    IUC *iuc = (IUC *) uf->d.v ;
    switch(m) {
    case UFM_TYPENAME:
	return(mtputs((MT *) a,"IRC")) ;
    case UFM_DESTROY:
	if (iuc->st) stimer_kill(iuc->st) ;
	close(iuc->efd->fd) ;
	free(iuc->in->s) ;
	dont_free(iuc) ;
	break ;
    case UFM_WAKEUP:
	irc_wakeup(uf) ;
	return(0) ;
    case UFM_ACTION:
	return(0) ;
    default:
	break ;
	}
    return(null_uff(uf,m,a)) ;
    }

/* ================================================================ */
static u32 irc_wakeup_st(STIMER *st,int m,u32 a)
{
    IUC *iuc = (IUC *) st->c ;
    switch(m) {
    case STM_EXPIRE:
	irc_uff(iuc->uf,UFM_WAKEUP,0) ;
	return(0) ;
	}
    return(0) ;
    }

static int irc_tid_max(IUC *c)
{
    char *s = "select max(id) from tannounce" ;
    int		v ;
    int		e ;
    if ((e = sql_single_row_int(sqlgp()->dbh,s,&v)) != SQLITE_ROW) v = 0 ;
    return(v) ;
    }

static int irc_last_pos(IUC *c)
{
    int		o = 0 ;
    return(o) ;
    }

/* there are various issues regarding transactions when code is driven from timers
   you could use separate database files
   you could use in memory databases
   you could use timers to schedule code to run from the main thread
   you could use semaphores or similar to protect critical code
*/

static void irc_catchup(IUC *c)
{
    int		o = irc_last_pos(c) ;
    int		fd = c->efd->fd ;
    int		n ;
    lseek(fd,o,SEEK_SET) ;
    c->tid_max = irc_tid_max(c) ;
    c->catchup = 1 ;
    transaction_begin() ;
    while ((n = irc_log_drain(c)) > 0) {
	}
    transaction_end() ;
    c->catchup = 0 ;
    }

static void rx_torrent_init(void)
{
    REGEX	*rx = iug->rx_torrent ;
    char *pat = "<Drone> \\(.*\\) - https://what.cd/torrents.php\\?id=\\(\\d+\\) / https://what.cd/torrents.php\\?action=download&id=\\(\\d+\\) - " ;
    rx->regexp = 1 ;
    re_compile_pattern_s(rx,pat) ;
    }

static void rx_time_init(void)
{
    REGEX	*rx = iug->rx_timestamp ;
    char *pat = "Session \\(Start\\|Time\\): \\(.*\\) " ;
    rx->regexp = 1 ;
    re_compile_pattern_s(rx,pat) ;
    }

/* ================================================================ */
#include	<pu/exithook.h>

static void iug_finish(void *a)
{
    if (!iug->init) return ;
    re_free(iug->rx_timestamp) ;
    re_free(iug->rx_torrent) ;
    }

static void iug_init(void)
{
    if (iug->init) return ;
    rx_torrent_init() ;
    rx_time_init() ;
    iug->init = 1 ;
    exithook_install(iug_finish,0) ;
    }

/* ================================================================ */
#ifdef __CYGWIN__
#define CDRIVE	"//boo/c"	
#else
#define CDRIVE	"/vom/c"	
#endif

#define IRCPATH	CDRIVE "/Documents and Settings/nick/Application Data/mIRC/logs/#what.cd-announce.What-Network.log"

static int iuc_anno_init(IUC *iuc)
{
    if (iuc->state & IUCS_INIT) return(0) ;
    iug_init() ;
{
    UF *uf = uf_create(irc_uff,iuc,0) ;
    EFD *efd = iuc->efd ;
    char *log = IRCPATH ;
    efd->fd = open(log,O_RDONLY) ;
    if (efd->fd == -1) {
	errorshow("cannot open IRC log file") ;
	return -1 ;
	}
{
    int flags = fcntl(efd->fd, F_GETFL, 0) ;
    fcntl(efd->fd, F_SETFL, flags | O_NONBLOCK) ;
    }
    
    mtmalloc(iuc->in,65536) ;
    iuc->parse[0] = iuc->in[0] ;

    iuc->uf = uf ;
    irc_catchup(iuc) ;
    iuc->state |= IUCS_INIT ;
    return 0 ;
    }
    }

static void irc_state_on(IUC *iuc)
{
    if (iuc_anno_init(iuc) == 0) {
	iuc->st = stimer_add(irc_wakeup_st,iuc,1000) ;
	iuc->st->state |= STS_REARM ;
	}
    }

static void irc_state_off(IUC *iuc)
{
    if (!iuc->st) return ;
    stimer_kill(iuc->st) ;
    iuc->st = 0 ;
    }

static void irc_state_toggle(IUC *iuc)
{
    if (iuc->st) 
	 irc_state_off(iuc) ;
    else irc_state_on(iuc) ;
    }

/* ================================================================ */
#include	"vcf.h"

extern u32 irc_state_vcf(void *c,int m,u32 a)
{
    IUC *iuc = iug->iuc_anno ;
    VCFU *u = (VCFU *) &a ;
    switch(m) {
    case VCF_SETCHAR:
	return(irc_state_vcf(c,switch_state_read(u->s),0)) ;
    case VCF_GETCHAR:
	switch_state_write(u->mt,iuc->st != 0) ;
	return 0 ;
    case VCF_SWITCH_ON:
	irc_state_on(iuc) ;
	return(0) ;
    case VCF_SWITCH_OFF:
	irc_state_off(iuc) ;
	return(0) ;
    case VCF_SWITCH_TOGGLE:
	irc_state_toggle(iuc) ;
	return(0) ;
    case VCF_GET_NAME:
	return(u32) "irc" ;
    case VCF_GET_STATE:
	return (u32) (iuc->st ? "on" : "off") ;
	}
    return(0) ;
    }

/* ================================================================ */
#include	"http.h"

/* ~# use http ; #~ */

#define IRC_VCC		DUMMY_VCC

/* ~~hpf(type => "state",help => "Set IRC poller state")~~ */
static u32 hpf_irc(HSR *r,int m,u32 a)
{
    switch (m) {
    case HPM_INTERNAL:
{
    char	*state_cmd ;
    if (!(state_cmd = hsr_arg_exists(r,"state"))) {
	state_cmd = "toggle" ;
	}
    vcf_apply(irc_state_vcf,IRC_VCC,VCF_SETCHAR,(u32) state_cmd) ;
    return(0) ;
    }
    case HPM_CONTENT_SEND:
	hsr_vcf_element_state(r,irc_state_vcf,IRC_VCC,"IRC") ;
	break ;
    default:
	break ;
	}
    return(hpf_generic_ok(r,m,a)) ;
    }

#include	".gen/ma_irc.c"

#ifdef __cplusplus /*Z*/
}
#endif
