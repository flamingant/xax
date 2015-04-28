#ifdef __cplusplus /*Z*/
extern "C" {
#endif

#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<stdarg.h>
#include	<unistd.h>

#include	<pu/mt.h>
#include	<time.h>
#include	<sys/time.h>

#include	"arg.h"
#include	"npu_util.h"

#include	"log.h"
/* ~# use collect ; collect::register_all('logsec','^LOGSEC (\w+)','$1') ; #~ */

typedef struct {
    FILE	*f ;
    char	*filename ;
    struct {
	FILE	*f ;
	char	*filename ;
	} t ;
    int		flags ;
    char	*tsf ;
    int		sequence ;
    } LOG_G ;

LOG_G log_g = {
    0,
    0,
    {},
    LF_STDOUT | LF_TIMESTAMP,
    "%H%M%S",
} ;

/* ================================================================ */
typedef struct {
    LOGSEC **s ;
    LOGSEC **e ;
    LOGSEC **c ;
    LOGSEC *ls ;
    } LOGSEC_ITC ;

extern LOGSEC **GSV_LOGSEC_start(void)
{
    extern LOGSEC *__start_GSV_LOGSEC[] ;
    return __start_GSV_LOGSEC ;
    }

extern LOGSEC **GSV_LOGSEC_end(void)
{
    extern LOGSEC *__stop_GSV_LOGSEC[] ;
    return __stop_GSV_LOGSEC ;
    }

extern void LOGSEC_ITC_init(LOGSEC_ITC *itc)
{
    itc->s = GSV_LOGSEC_start() ;
    itc->e = GSV_LOGSEC_end() ;
    itc->c = itc->s ;
    }

static LOGSEC *ls_lookup(char *name)
{
    LOGSEC **s = GSV_LOGSEC_start() ;
    LOGSEC **e = GSV_LOGSEC_end() ;
    LOGSEC **ls ;
    for (ls = s ; ls < e ; ls++)
	if (!stricmp((*ls)->name,name)) return *ls ;
    return 0 ;
    }

static LOGSEC *ls_lookup_partial(char *name,char *prefix)
{
    int		n = strlen(prefix) ;
    if (!strncmp(name,prefix,n)) return(ls_lookup(name+n)) ;
    return(0) ;
    }

/* ================================================================ */

extern void log_prefix_set(int n)
{
    log_g.flags &= ~(LF_TIMESTAMP | LF_SEQUENCE) ;
    log_g.flags |= n ;
    }

extern void log_close(void)
{
    if (log_g.f && (log_g.f != stdout) && (log_g.f != stderr))
	fclose(log_g.f) ;
    log_g.f = 0 ;
    }

extern void log_set(FILE *f)
{
    log_close() ;
    log_g.f = f ;
    }

extern void log_open(char *file)
{
    FILE *f = fopen(file,"w") ;
    log_set(f) ;
    }

static void mtput_timestamp(MT *mt,char *tsf)
{
    struct timeval tv[1] ;
    struct tm	tm[1] ;
    if (!tsf) tsf = "%y%m%d-%H%M%S" ;
    gettimeofday(tv, NULL) ;
    localtime_r(&tv->tv_sec,tm) ;
    strftime(mt->s,MTGapSize(mt),tsf,tm) ;
    mtadvance(mt,strlen(mt->s)) ;
    }

static char *timestamp_filename(char *tsf,char *suffix)
{
    MT mt[1] ;
    mtmalloc(mt,32) ;
    mtput_timestamp(mt,0) ;
    mtputs(mt,suffix) ;
    return  mt->s ;
    }

/* ================================================================ */
/* ~# use arg ; #~ */

/* ~~argset(help => "Logging settings",omf => 'argset_omf')~~ */

#include	".gen/log.h"

extern void log_filename_set(char *name)
{
    log_g.flags = (log_g.flags & ~(LF_STDOUT | LF_STDERR)) ;
    log_g.filename = name ;
    }

/* ~~arg(help => "Log to standard output (stdout)",type => "bool")~~ */
static int argf__log_stdout(char *name,char *value,void *a0)
{
    log_g.flags = (log_g.flags & ~(LF_STDOUT | LF_STDERR)) | LF_STDOUT ;
    return(ASF_ARGACCEPTED | ASF_VALUEIGNORED) ;
}

/* ~~arg(help => "Log to standard error (stderr)",type => "bool")~~ */
static int argf__log_stderr(char *name,char *value,void *a0)
{
    log_g.flags = (log_g.flags & ~(LF_STDOUT | LF_STDERR)) | LF_STDERR ;
    return(ASF_ARGACCEPTED | ASF_VALUEIGNORED) ;
}

/* ~~arg(help => "Log to named file",value => "FILE")~~ */
static int argf__log_file(char *name,char *value,void *a0)
{
    log_filename_set(strdup(value)) ;
    return(ASF_ARGACCEPTED) ;
}

/* ~~arg(help => "Tee output to named file",value => "FILE")~~ */
static int argf__log_tee_file(char *name,char *value,void *a0)
{
    log_g.t.filename = strdup(value) ;
    return(ASF_ARGACCEPTED) ;
}

/* ~~arg(help => "Log to timestamp named file",type => 'bool')~~ */
static int argf__log_file_auto(char *name,char *value,void *a0)
{
    log_filename_set(timestamp_filename(0,".log")) ;
    return(ASF_ARGACCEPTED | ASF_VALUEIGNORED) ;
}

/* ~~arg(help => "Tee output to timestamp named file",type => 'bool')~~ */
static int argf__log_tee_file_auto(char *name,char *value,void *a0)
{
    log_g.t.filename = timestamp_filename(0,".log") ;
    return(ASF_ARGACCEPTED | ASF_VALUEIGNORED) ;
}

/* ~~arg(help => "Include date in log output",type => "bool")~~ */
static int argf__log_date(char *name,char *value,void *a0)
{
    if (strtol(value,0,10))
	log_g.tsf = "%y%m%d-%H%M%S" ;
    return(ASF_ARGACCEPTED | ASF_VALUEIGNORED) ;
}

/* ~~arg(help => "Set log timestamp format",value => "FORMAT")~~ */
static int argf__log_time_format(char *name,char *value,void *a0)
{
    log_g.tsf = strdup(value) ;
    return(ASF_ARGACCEPTED) ;
}

/* ~~arg(help => "Set log prefix to sequence number",value => "bool")~~ */
static int argf__log_prefix_sequence(char *name,char *value,void *a0)
{
    if (strtol(value,0,10))
	log_prefix_set(LF_SEQUENCE) ;
    return(ASF_ARGACCEPTED) ;
}

/* ================================================================ */
#include	"vcf.h"

/* ~~arg(help => "List all logging sections",type => "bool")~~ */
static int argf__log_section_list(char *name,char *value,void *a0)
{
    LOGSEC_ITC	itc[1] ;
    int		vcf = switch_state_read(value) ;
    if (vcf == VCF_SWITCH_OFF) goto done ; 
    for (LOGSEC_ITC_init(itc) ; itc->c < itc->e ; itc->c++) {
	LOGSEC *s = *itc->c ;
	printf("%1s %-16s %s\n",
	       s->enable ? "+" : " ",
	       s->name,
	       s->description ? s->description : "") ;
	}
    fflush(stdout) ;
done :
    return(ASF_ARGACCEPTED | ASF_VALUEIGNORED) ;
}

/* ~~arg(help => "All logging sections switch on/off",type => "bool")~~ */
static int argf__log_section_all(char *name,char *value,void *a0)
{
    LOGSEC_ITC	itc[1] ;
    int		vcf = switch_state_read(value) ;
    for (LOGSEC_ITC_init(itc) ; itc->c < itc->e ; itc->c++) {
	LOGSEC *s = *itc->c ;
	switch (vcf) {
	case VCF_SWITCH_ON:	s->enable = 1 ; break ;
	case VCF_SWITCH_OFF:	s->enable = 0 ; break ;
	case VCF_SWITCH_TOGGLE:	s->enable ^= 1 ; break ;
	}
	}
    return(ASF_ARGACCEPTED | ASF_VALUEIGNORED) ;
}

static int argtry(ARGSET *o,ALI *a)
{
    ARGITEM *i = argitem_find(o->items,a->ps) ;
    if (i) return i->accept(a->ps,a->pv,a) ;
{
    LOGSEC *ls ;
    if (ls = ls_lookup_partial(a->ps,"log-section-")) {
	switch(switch_state_read(a->pv)) {
	case VCF_SWITCH_ON:	ls->enable = 1 ; break ;
	case VCF_SWITCH_OFF:	ls->enable = 0 ; break ;
	case VCF_SWITCH_TOGGLE:	ls->enable ^= 1 ; break ;
	case VCF_UNKNOWN:	/* try other types */
	    break ;
	    }
	return ASF_ARGACCEPTED ;
	}
    }
    return(ASF_ARGIGNORED) ;
}

static u32 argset_omf(ARGSET *o,int m,u32 a)
{
    switch(m) {
	case ARGM_ARGTRY:	return(argtry(o,(ALI *) a)) ;
	}
    return(argset_omf_null(o,m,a)) ;
    }

extern int log_init(int argc,char **argv)
{
    argc = argset_try_one(argc,argv,argset_log) ;

    if (log_g.t.filename && !(log_g.t.f = fopen(log_g.t.filename,"w"))) {
	errorshow("Cannot open log-tee-file %s\n",log_g.t.filename) ;
	log_g.flags |= LF_OPENFAIL ;
	return -1 ;
	}
    if (log_g.filename && !(log_g.f = fopen(log_g.filename,"w"))) {
	errorshow("Cannot open log-file %s\n",log_g.t.filename) ;
	log_g.flags |= LF_OPENFAIL ;
	return -1 ;
	}
    else if (log_g.flags & LF_STDOUT) {
	log_g.f = stdout ;
	}
    else if (log_g.flags & LF_STDERR) {
	log_g.f = stderr ;
	}

    return argc ;
    }
	
/* ================================================================ */
static void log_vprintf_c_(LOGSEC *ls,char *fmt,va_list va)
{
    if (log_g.f) {
	vfprintf(log_g.f,fmt,va) ;
	fflush(log_g.f) ;
	}
    if (log_g.t.f) {
	vfprintf(log_g.t.f,fmt,va) ;
	fflush(log_g.t.f) ;
	}
    }

/* ================================================================ */
static int log_enabled(void)
{
    if (log_g.f) return(1) ;
    if (log_g.flags & LF_OPENFAIL) return(0) ;
    log_init(0,0) ;
    return(log_enabled()) ;
    }

extern void log_vprintf_c(LOGSEC *ls,char *fmt,va_list va)
{
    if (!ls->enable || !log_enabled()) return ;
    log_vprintf_c_(ls,fmt,va) ;
    }

extern void log_printf_c(LOGSEC *ls,char *fmt,...)
{
    va_list	va ;
    va_start(va,fmt) ;
    log_vprintf_c(ls,fmt,va) ;
    }

extern void log_puts_c(LOGSEC *ls,char *s)
{
    if (!ls->enable || !log_enabled()) return ;
    log_write_s(ls,s) ;
    }

extern void log_write(LOGSEC *ls,char *pb,int cb)
{
    if (!ls->enable || !log_enabled()) return ;
    if (log_g.f) {
	write(fileno(log_g.f),pb,cb) ;
	fflush(log_g.f) ;
	}
    if (log_g.t.f) {
	write(fileno(log_g.t.f),pb,cb) ;
	fflush(log_g.t.f) ;
	}
    }

extern void log_write_s(LOGSEC *ls,char *s)
{
    log_write(ls,s,strlen(s)) ;
    }

/* ================================================================ */
extern void mt_put_timestamp(MT *mt)
{
    struct timeval tv[1] ;
    struct tm	tm[1] ;
    gettimeofday(tv, NULL) ;
    localtime_r(&tv->tv_sec,tm) ;
    strftime(mt->s,MTGapSize(mt),log_g.tsf,tm) ;
    mtadvance(mt,strlen(mt->s)) ;
    mtprintf(mt,".%03d ",(tv->tv_usec / 1000)) ;
    }

static void log_timestamp(LOGSEC *ls)
{
    MT mt[1] ;
    MTALLOCA(mt,256) ;
    mt_put_timestamp(mt) ;
    log_write(ls,mt->s,MTGapOffset(mt)) ;
    }

static void log_line_start(LOGSEC *ls)
{
    if (log_g.flags & LF_TIMESTAMP)
	log_timestamp(ls) ;
    if (log_g.flags & LF_SEQUENCE)
	log_printf_c(ls,"%05d:",log_g.sequence++) ;
    if (ls->name)
	log_printf_c(ls,"%s:",ls->name) ;
    }

extern void log_puts(LOGSEC *ls,char *s)
{
    if (!ls->enable || !log_enabled()) return ;
    log_line_start(ls) ;
    log_write_s(ls,s) ;
    }

extern void log_vprintf(LOGSEC *ls,char *fmt,va_list va)
{
    if (!ls->enable || !log_enabled()) return ;
    log_line_start(ls) ;
    log_vprintf_c_(ls,fmt,va) ;
    }

extern void log_printf(LOGSEC *ls,char *fmt,...)
{
    va_list	va ;
    va_start(va,fmt) ;
    log_vprintf(ls,fmt,va) ;
    }

extern void log_put_escape_nl(LOGSEC *ls,char *s,int len)
{
    MT	mts[1] ;
    MT	mtd[1] ;
    MTSETN(mts,s,len) ;
    MTALLOCA(mtd,len*2) ;
    mtcpymt_all_escape_nl(mtd,mts) ;
    log_puts(ls,mtd->s) ;
    }

extern void log_puts_escape_nl(LOGSEC *ls,char *s)
{
    log_put_escape_nl(ls,s,strlen(s)) ;
}

extern void log_put_to_first_nl(LOGSEC *ls,char *s,int len)
{
    MT	mts[1] ;
    MT	mtd[1] ;
    MTSETN(mts,s,len) ;
    MTALLOCA(mtd,len*2) ;
    mtcpymt_to_first_nl(mtd,mts) ;
    log_puts(ls,mtd->s) ;
    }

extern void log_puts_to_first_nl(LOGSEC *ls,char *s)
{
    log_put_to_first_nl(ls,s,strlen(s)) ;
    }

LOGSEC ls_generic[] = {"",1} ;

/* ================================================================ */
#include	<pu/printg.h>

extern void log_printg_va(LOGSEC *ls,char *fmt,va_list va)
{
    if (!ls->enable || !log_enabled()) return ;
    if (log_g.flags & LF_TIMESTAMP)
	log_timestamp(ls) ;
    if (log_g.f) {
	fprintg_va(log_g.f,fmt,va) ;
	fflush(log_g.f) ;
	}
    if (log_g.t.f) {
	fprintg_va(log_g.t.f,fmt,va) ;
	fflush(log_g.t.f) ;
	}
    }

extern void log_printg(LOGSEC *ls,char *fmt,...)
{
    va_list	va ;
    va_start(va,fmt) ;
    log_printg_va(ls,fmt,va) ;
    }

#include	".gen/log.c"

#ifdef __cplusplus /*Z*/
}
#endif
