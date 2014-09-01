#ifdef __cplusplus /*Z*/
extern "C" {
#endif

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

#include	<stdarg.h>
#include	"pu/mt.h"

#include	"pu/rcons.h"
#include	"pu/regex.h"
#include	"pu/sk.h"
#include	"pu/rat.h"

#include	"uf.h"

#include	"arg.h"

#include	"npu_util.h"

#define UF_ARG_WANT_ALL	1

#include	"uf_arg.h"

#include	".gen/uf.h"

static struct {
    RCONS	*uf_poll ;
    RCONS	*uf_reg ;
    int		quit ;
    struct {
	struct timeval tv ;
	int	min_open ;
	int	max_open ;
	int	max_fd ;
	fd_set	*rfd ;
	fd_set	*wfd ;
	UF	**uf ;
	} sel ;
    int		ufm_count ;
    int		max_poll ;

    RAY		ufq_ray[1] ;
    } ufs ;

extern u32 uf_send_direct(UF *uf,int m,u32 a)
{
    return(uf->f(uf,m,a)) ;
    }

#define UF_SEND_RAW(u,m,a)	((u)->f((u),m,a))

/* ================================================================ */
extern int ufm_offset(UFM m)
{
    if (m < UFM__STD_AFTERLAST)
	return(m) ;
    if (m < UFM__HT_AFTERLAST)
	return(m - UFM__HT_FIRST + UFM__STD_AFTERLAST) ;

    errorfatal("unregistered message %d",m) ;
    return(-1) ;
    }

/* ================================================================ */
static char *ufm_name_base(int m)
{
    char *c = (char *) glget(glt_enum_UFM,(GLT_CAR_T) m) ;
    if (c) return(c) ;
    return("*unknown*") ;
    }

extern SK *sk_rassoc(SK *sk,void *info,int (*fun)(void *,void *))
{
    if (!sk) return(sk) ;
    if (fun) {
	while (1) {
	    if (!sk->info) return(0) ;
	    if (!fun(sk->info,info)) break ;
	    sk++ ;
	    }
	}
    else {
	while (1) {
	    if (!sk->info) return(0) ;
	    if (sk->info == info) break ;
	    sk++ ;
	    }
	}
    return(sk) ;
    }

/* ================================================================ */
static int ufm_arg_describe_base(UF *f,int m,u32 a,MT *mt)
{
    switch(m) {
    case UFM_CREATE:
    case UFM_DESTROY:
    case UFM_READ:
    case UFM_CONNECT:
    case UFM_CONNECT_OK:
    case UFM_SELECT_OK:
    case UFM_WAKEUP:

    case UFM_HT_RESPONSE_BODY:
    case UFM_DESCRIBE:
    case UFM_QUEUE_ADD:
    default:
	mtprintf(mt,"%x ",a) ;
    }
    return(0) ;
    }

extern int UFM_ARG_DESCRIBE_apply(int (*fun)(UF *,int,u32,MT *),UF *f,u32 a)
{
    UFM_ARG_DESCRIBE_ARG *aa = (typeof(aa)) a ;
    return(fun(f,aa->m,aa->a,aa->mt)) ;
    }

extern int UFM_ARG_DESCRIBE_send(UF *f,int m,u32 a,MT *mt)
{
    UFM_ARG_DESCRIBE_ARG	aa[1] ;
    aa->mt = mt ;
    aa->m = m ;
    aa->a = a ;
    return UF_SEND_RAW(f,UFM_ARG_DESCRIBE,(u32) aa) ;
    }

/* ~# use collect ; collect::register('uff','null_uff') ; #~ */

UFSS null_ufss[] = {"null"} ;

extern u32 null_uff(UF *f,int m,u32 a)
{
    switch(m) {
    case UFM_GET_STATIC:
	return (u32) null_ufss ;
    case UFM_MSG_NAME:
	return((u32) ufm_name_base(a)) ;
    case UFM_TRACE_OK:
	if (f->trace.all > f->trace.none) return 1 ;
	if (f->trace.all < f->trace.none) return 0 ;
	if (null_ufss->trace.all > null_ufss->trace.none) return 1 ;
	if (null_ufss->trace.all < null_ufss->trace.none) return 0 ;
	return(f->trace.ufm[(int) ufm_offset((UFM) a)]) ;
    case UFM_TYPENAME:
	mtprintf((MT *) a,"((UFF *) 0x%08x)",f->f) ;
	return(0) ;
    case UFM_DESCRIBE:
	return(UF_SEND_RAW(f,UFM_TYPENAME,a)) ;
    case UFM_ARG_DESCRIBE:
	return(UFM_ARG_DESCRIBE_apply(ufm_arg_describe_base,f,a)) ;
    default:
	return(0) ;
	}
    }

UF uf_dummy[] = {null_uff,0}  ;

extern int ufss_trace_arg_try(UFSS *ss,char *s,int cmd)
{
    char	*msgname = s ;
    u8		*p ;

    if (!strcmp(msgname,"all"))
	p = &ss->trace.all ;
    else if (!strcmp(msgname,"none"))
	p = &ss->trace.none ;
    else if (!strcmp(msgname,"basic")) {
	ufss_trace_arg_try(ss,"UFM_CREATE",cmd) ;
	ufss_trace_arg_try(ss,"UFM_DESTROY",cmd) ;
	ufss_trace_arg_try(ss,"UFM_SELECT_OK",cmd) ;
	ufss_trace_arg_try(ss,"UFM_DISCONNECT_IND",cmd) ;
	return(1) ;
	}
    else {
	GLT	*glt ;
	glt = glrassoc(glt_enum_UFM,(GLT_CDR_T) msgname,(GLTCMP) strcmp) ;
	if (!glt) return(0) ;
	p = ss->trace.ufm + ufm_offset((UFM) (int) glt->car) ;
	}

    switch(cmd) {
    case 0:	*p &= ~1 ; break ;
    case 1:	*p |=  1 ; break ;
    case 2:	*p ^=  1 ; break ;
    }
    return(1) ;
    }

extern UFF uff_initvec[] ;

extern UFSS *ufss_lookup(char *name)
{
    int		i ;
    for (i = 0 ; uff_initvec[i] ; i++) {
	UFF f = uff_initvec[i] ;
	UFSS *ss = (UFSS *) f(uf_dummy,UFM_GET_STATIC,0) ;
	if (ss && !stricmp(ss->name,name)) return ss ;
	}
    return 0 ;
    }

#include	"gmalloc.h"

extern int trace_arg_try(char *s,int cmd)
{
    UFSS	*ss = null_ufss ;
    char	*at ;
    if (at = (strchr(s,'@'))) {
	GMC *m = mt_malloc_open(64) ;
	char *ssn = gstrdup(m,s) ;
	char *z = ssn + (at - s) ;
	*z = 0 ;
	if (!(ss = ufss_lookup(ssn))) {
	    errorfatal("unknown UFMSID (%s)\n",ssn) ;
	    }
	gmalloc_close(m) ;
	s = at+1 ;
	}
    return(ufss_trace_arg_try(ss,s,cmd)) ;
    }

/* ================================================================ */
/* ~# use arg ; #~ */

static void uf_trace_argset_gethelp(ARGSET *o,MT *mt)
{
    mtprintf(mt,"  UFMSID is one of:\n      all\n     none\n    basic - UFM_CREATE UFM_DESTROY and UFM_SELECT_OK\n") ;
    mtprintf(mt,"      MMM - standard message name MMM\n") ;
    mtprintf(mt,"    X@MMM - module specific message MMM belonging to module X\n\n") ;
    }

static u32 uf_trace_argset_omf(ARGSET *o,int m,u32 a)
{
    switch(m) {
	case OM_GETHELP:
	    argset_omf_null(o,m,a) ;
	    uf_trace_argset_gethelp(o,(MT *) a) ;
	    return(0) ;
	}
    return(argset_omf_null(o,m,a)) ;
    }

/* ~~argset(
   help => "UF message trace settings",
   omf => uf_trace_argset_omf,
   )~~ */

/* ~~arg(help => "Enable trace",value => "UFMSID")~~ */
static int argf__trace(char *name,char *value,void *a0)
{
    trace_arg_try(value,1) ;
    return(ASF_ARGACCEPTED) ;
}

/* ~~arg(help => "Disable trace",value => "UFMSID")~~ */
static int argf__notrace(char *name,char *value,void *a0)
{
    trace_arg_try(value,0) ;
    return(ASF_ARGACCEPTED) ;
}

/* ~~arg(help => "Toggle trace enable state",value => "UFMSID")~~ */
static int argf__xortrace(char *name,char *value,void *a0)
{
    trace_arg_try(value,2) ;
    return(ASF_ARGACCEPTED) ;
}

/* ================================================================ */
static int ufs_long_arg_fun(char *name,char *value,void *a0)
{
    return(argitem_try(args_uf,name,value,a0)) ;
    }

/* ================================================================ */
#include	"log.h"

/* ~# use collect ; collect::register_all('logsec','^LOGSEC (\w+)','$1') ; #~ */

LOGSEC lc_uf[] = {0,1} ;

static void uf_diag(MT *mto,UF *uf,char *format)
{
    MT		mt[1] ;
    MTALLOCA(mt,1024) ;
    uf->f(uf,UFM_DESCRIBE,(u32) mt) ;
    mtprintf(mto,format,mt->s) ;
    }

static char *ufm_name(UF *f,int m)
{
    return (char *) UF_SEND_RAW(f,UFM_MSG_NAME,m) ;
    }
    
static void ufm_msg_name(UF *f,int m,MT *mt)
{
    mtprintf(mt,"%-24s ",ufm_name(f,m)) ;
    }

static int ufm_arg_describe(UF *f,int m,u32 a,MT *mt)
{
    return(UFM_ARG_DESCRIBE_send(f,m,a,mt)) ;
}

static void uf_send_trace(UF *uf,int m,u32 a)
{
    MT		mt[1] ;
    MTALLOCA(mt,1024) ;
    UF_SEND_RAW(uf,UFM_DESCRIBE,(u32) mt) ;
    mtputs(mt,": ") ;
    ufm_msg_name(uf,m,mt) ;
    ufm_arg_describe(uf,m,a,mt) ;
    log_printf(lc_uf,"%s\n",mt->s) ;
    }

extern u32 uf_send(UF *uf,int m,u32 a)
{
    if (UF_SEND_RAW(uf,UFM_TRACE_OK,m)) {
	uf_send_trace(uf,m,a) ;
	}
    return(uf->f(uf,m,a)) ;
    }

/* ================================================================ */
static UFQ *ufq_alloc(void)
{
    return ray_ualloc(ufs.ufq_ray) ;
}

static void ufq_free(UFQ *q)
{
    ray_free(ufs.ufq_ray,q) ;
}

extern void uf_queue_tail(UF *uf,int m,u32 a)
{
    UFQ		*q = ufq_alloc() ;
    q->m = m ;
    q->a = a ;
    q->next = 0 ;
    *uf->queue.tail = q ;
    uf->queue.tail = &q->next ;
    }

extern void uf_queue(UF *uf,int m,u32 a)
{
    if (!uf->queue.head) uf_queue_tail(uf,m,a) ;
    return ;
{
    UFQ		*q = ufq_alloc() ;
    q->m = m ;
    q->a = a ;
    q->next = uf->queue.head ;
    uf->queue.head = q ;
    }
    }

extern void uf_unqueue_head(UF *uf)
{
    UFQ		*q = uf->queue.head ;
    if (!(uf->queue.head = q->next)) uf->queue.tail = &uf->queue.head ;
    ufq_free(q) ;
    }

/* ================================================================ */
extern void uf_trace_va(UF *uf,char *fmt,va_list va)
{
    MT		mt[1] ;
    MTALLOCA(mt,8192) ;
    mt_put_timestamp(mt) ;
    uf->f(uf,UFM_DESCRIBE,(u32) mt) ;
    mtputs(mt,": ") ;
    mtvprintf(mt,fmt,va) ;
    log_write(ls_generic,mt->s,MTFillSize(mt)) ;
    }

extern void uf_trace(UF *uf,char *fmt,...)
{
    va_list	va ;
    va_start(va,fmt) ;
    uf_trace_va(uf,fmt,va) ;
    }

/* ================================================================ */
extern u32 uf_parent_notify(UF *uf,int nm,int m,u32 a)
{
    if (!uf->parent) return(0) ;
{
    UFM_NOTIFY_ARG	aa[1] ;
    aa->uf = uf ;
    aa->m = m ;
    aa->a = a ;
    return(UF_SEND_RAW(uf->parent,nm,(u32) aa)) ;
    }
    }

extern u32 uf_send_and_notify(UF *uf,int m,u32 a)
{
    if (!uf->parent) return(uf_send(uf,m,a)) ;
{
    UFM_NOTIFY_ARG	aa[1] ;
    u32		r ;
    aa->uf = uf ;
    aa->m = m ;
    aa->a = a ;
    UF_SEND_RAW(uf->parent,UFM_PRE_NOTIFY,(u32) aa) ;
    r = uf_send(uf,m,a) ;
    UF_SEND_RAW(uf->parent,UFM_POST_NOTIFY,(u32) aa) ;
    return r ;
    }
    }

/* ================================================================ */
#define uf_alloc()	tscalloc(1,UF)
#define uf_free(u)	free(u)

extern UF *uf_create(UFF f,void *d,void *cp)
{
    UF *uf = uf_alloc() ;
    uf->f = f ;
    uf->d.v = d ;
    uf->state = 0 ;
    uf->parent = 0 ;
    uf->queue.head = 0 ;
    uf->queue.tail = &uf->queue.head ;
{
    UFSS	*ss = (UFSS *) uf_send_direct(uf,UFM_GET_STATIC,0) ;
    if (ss) uf->trace = ss->trace ;
    }
    uf_send(uf,UFM_CREATE,(u32) cp) ;
    ufs.uf_reg = rcons_cons(uf,ufs.uf_reg) ;
    return(uf) ;
    }

static void uf_destroy_no_unlink(UF *uf)
{
    uf_send_and_notify(uf,UFM_DESTROY,0) ;
    uf_free(uf) ;
    }

extern void uf_destroy(UF *uf)
{
    rcons_delete(uf,&ufs.uf_reg) ;
    while (uf->queue.head)
	uf_unqueue_head(uf) ;
    uf_destroy_no_unlink(uf) ;
    }

extern void uf_socket_register(UF *uf,int fd)
{
    if (fd > ufs.sel.max_fd) {
	int	n = FD_SETSIZE * ((fd / FD_SETSIZE) + 1) ;
	int	size = n / NBBY ; 
	ufs.sel.rfd = (fd_set *) realloc(ufs.sel.rfd,size) ;
	ufs.sel.uf = (UF **) realloc(ufs.sel.uf,n * sizeof(UF *)) ;
	while (++ufs.sel.max_fd < n) {
	    FD_CLR(ufs.sel.max_fd,ufs.sel.rfd) ;
	    ufs.sel.uf[ufs.sel.max_fd] = 0 ;
	    }
    }
    FD_SET(fd,ufs.sel.rfd) ;
    if (fd < ufs.sel.min_open) {
	ufs.sel.min_open = fd ;
	}
    if (fd > ufs.sel.max_open) {
	ufs.sel.max_open = fd ;
	}
    ufs.sel.uf[fd] = uf ;
    }

extern void uf_socket_unregister(UF *uf,int fd)
{
    FD_CLR(fd,ufs.sel.rfd) ;
    ufs.sel.uf[fd] = 0 ;
    if (fd == ufs.sel.max_open) {
	while (ufs.sel.max_open > ufs.sel.min_open && !ufs.sel.uf[--ufs.sel.max_open]) ;
	}
    if (fd == ufs.sel.min_open) {
	if (ufs.sel.max_open == ufs.sel.min_open) {
	    ufs.sel.min_open = NOFILE+1 ;
	    ufs.sel.max_open = -1 ;
	    }
	else 
	    while (++ufs.sel.min_open <= ufs.sel.max_open && !ufs.sel.uf[ufs.sel.min_open]) ;
	}
    }

/* ================================================================ */
extern void uf_map_rcons(RCONS *r,void (*fun)(UF *))
{
    for ( ; r ; r = r->cdr) {
	UF *m = (UF *) r->car ;
	fun(m) ;
	}
    }

extern void uf_map1_rcons(RCONS *r,void (*fun)(UF *,u32),u32 a)
{
    for ( ; r ; r = r->cdr) {
	UF *m = (UF *) r->car ;
	fun(m,a) ;
	}
    }

extern UF *uf_scan_rcons(RCONS *r,int (*fun)(UF *))
{
    for ( ; r ; r = r->cdr) {
	UF *m = (UF *) r->car ;
	if (fun(m)) return(m) ;
	}
    return(0) ;
    }

extern UF *uf_scan1_rcons(RCONS *r,int (*fun)(UF *,u32),u32 a)
{
    for ( ; r ; r = r->cdr) {
	UF *m = (UF *) r->car ;
	if (fun(m,a)) return(m) ;
	}
    return(0) ;
    }

/* ================================================================ */
#include	<pu/printg.h>

static int pff_uf(char *pb,int cb,FMTSPEC *spec,UF *uf)
{
    MT		mt[1] ;
    MTSETN(mt,pb,cb) ;
    cb = uf_send_direct(uf,UFM_DESCRIBE,(u32) mt) ;
    return(cb) ;
    }

/* ================================================================ */
extern void ufs_rc_add(UF *uf)
{
    uf_send(uf,UFM_QUEUE_ADD,0) ;
    ufs.uf_poll = rcons_tail_cons(uf,ufs.uf_poll) ;
    }

static void uf_select_ok(UF *uf,int type)
{
    uf_send(uf,UFM_SELECT_OK,type) ;
    }

static void ufs_select_scan(fd_set *set,int type)
{
    int i ;
    if (!set) return ;
    for (i = ufs.sel.min_open ; i <= ufs.sel.max_open ; i++) {
	if (FD_ISSET(i,set))
	    uf_select_ok(ufs.sel.uf[i],type) ;
	}
    }

static void ufs_select(void)
{
    int s ;
    fd_set	*rfd = 0 ;
    fd_set	*wfd = 0 ;
    fd_set	*efd = 0 ;
    int size = ufs.sel.max_fd / NBBY ;
    if (ufs.sel.rfd) {
	rfd = (fd_set *) alloca(size) ;
	memcpy(rfd,ufs.sel.rfd,size) ;
	}
    if (ufs.sel.wfd) {
	wfd = (fd_set *) alloca(size) ;
	memcpy(wfd,ufs.sel.wfd,size) ;
	}
    if (ufs.sel.rfd || ufs.sel.wfd) {
	efd = (fd_set *) alloca(size) ;
	memcpy(efd,ufs.sel.rfd,size) ;
	/* xor wfd flags here */
	}
	
    s = select(ufs.sel.max_open+1,rfd,wfd,efd,&ufs.sel.tv) ;
    if (s == -1) {
	return ;
	}
    if (s == 0) {
	return ;
	}
    ufs_select_scan(rfd,0) ;
    ufs_select_scan(wfd,1) ;
    ufs_select_scan(efd,2) ;
    }

extern void ufs_loop(void)
{
    while (!ufs.quit) {
	RCONS	*r,**rr = &ufs.uf_poll ;
	int	n = ufs.max_poll ;
	while ((r = *rr) != 0 && n-- > 0) {
	    UF *uf = (UF *) r->car ;
	    if (uf->queue.head) {
		uf_send(uf,uf->queue.head->m,uf->queue.head->a) ;
		}
	    if (uf->state & UFS_DESTROY) {
		uf_destroy(uf) ;
		*rr = r->cdr ;
		rcons_free(r) ;
		}
	    else {
		rr = &r->cdr ;
		}
	    }
	ufs_select() ;
	fflush(stdout) ;
	}
    }

extern int ufs_init(int argc,char **argv)
{
    ufs.max_poll = 16 ;
    ufs.sel.min_open = NOFILE+1 ;
    ufs.sel.max_open = -1 ;
    ufs.sel.max_fd = -1 ;
    ufs.sel.tv.tv_sec = 0 ;
    ufs.sel.tv.tv_usec = 1000 ;
    ufs.ufm_count = UFM__COUNT ;
    argc = arg_read_simple(argc,argv,ufs_long_arg_fun,0) ;
    printg_register("uf",(PFF_S) pff_uf) ;
    ufs.ufq_ray->rat->unit_size = sizeof(UFQ) ;
    ufs.ufq_ray->rat->units_per_node = 128 ;
    ray_init(ufs.ufq_ray) ;
    return(argc) ;
    }

extern void ufs_destroy(void)
{
    while (ufs.uf_reg) {
	uf_destroy_no_unlink((UF *) ufs.uf_reg->car) ;
        ufs.uf_reg = rcons_free(ufs.uf_reg) ;
	}
    rcons_free_list(ufs.uf_poll) ;
    freenz(ufs.sel.rfd) ;
    freenz(ufs.sel.wfd) ;
    freenz(ufs.sel.uf) ;
    ray_destroy(ufs.ufq_ray) ;
    }

extern void ufs_quit(void)
{
    ufs.quit = 1 ;
    }

#include	".gen/uf.c"

#ifdef __cplusplus /*Z*/
}
#endif
