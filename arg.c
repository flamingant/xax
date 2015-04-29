#ifdef __cplusplus /*Z*/
extern "C" {
#endif

#include	<unistd.h>
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<malloc.h>

#include	<pu/hash.h>

#include	"arg.h"

#include	".gen/arg.h"

ARG_G arg_g ;

/* ================================================================ */
extern ARGSET **GSV_ARGSET_start(void)
{
    extern ARGSET *__start_GSV_ARGSET[] ;
    return __start_GSV_ARGSET ;
    }

extern ARGSET **GSV_ARGSET_end(void)
{
    extern ARGSET *__stop_GSV_ARGSET[] ;
    return __stop_GSV_ARGSET ;
    }

/* ================================================================ */
extern int ali_as_try(ALI *z,ARGSET **as,ARGSET **as_end)
{
    int asf ;
    for ( ; as < as_end ; as++) {
	asf = (*as)->omf(*as,ARGM_ARGTRY,(u32) z) ;
	if (asf != ASF_ARGIGNORED) return asf ;
	}
    return(ASF_ARGUNKNOWN) ;
    }
    
extern int ali_as_all_try(ALI *z)
{
    return(ali_as_try(z,GSV_ARGSET_start(),GSV_ARGSET_end())) ;
    }
    
/* ================================================================ */
extern void arg_copy(char **from,char *to)
{
    if (*from) free(*from) ;
    *from = strdup(to) ;
    }

static void ali_unpack(ALI *z,int argc,char **argv)
{
    z->ps += 2 ;
    z->pv = z->ps ;
    z->n = 1 ;

    while (*++z->pv) {
	if (*z->pv == '=') {*z->pv = 0 ; z->pv++ ; break ;}
    }

    if (!*z->pv) {
	if (argc > 1) {
	    z->pv = *(argv+1) ;
	    z->n = 2 ;
	    }
	else {
	    z->pv = "" ;
	    }
	}
    }

extern int arg_long_or(int what,int argc,char **argv,int (*fun)(char *,char *,void *),void *a0)
{
    char *arg = (char *) alloca(strlen(*argv)+1) ;
    strcpy(arg,*argv) ;
{
    int asf ;
    ALI		z ;
    z.ps = arg ;
    ali_unpack(&z,argc,argv) ;

    if ((asf = fun(z.ps,z.pv,a0)) != ASF_ARGIGNORED) {
	if (*z.pv && (asf & ASF_VALUEIGNORED))
	    if (z.n == 2)
		 z.n = 1 ;
	    else errorprintf("value '%s' attached to parameter '%s' ignored",z.pv,z.ps) ;
	while (z.n-- > 0) *argv++ = 0 ;
	return(1) ;
	}
    else {
	errorshow_and(what,"unrecognized argument '%s'\n",z.ps) ;
	return(0) ;
    }
    }
 }
    
extern int arg_long(int argc,char **argv,int (*fun)(char *,char *,void *),void *a0)
{
    return(arg_long_or(OR_NOTHING,argc,argv,fun,a0)) ;
    }

extern void arg_dump(int argc, char **argv, int show_missing)
{
    int		i ;
    for (i = 1 ; i < argc ; i++) {
	char *arg = argv[i] ;
	if (arg)
	    printf("\"%s\" ",arg) ;
	else if (show_missing)
	    printf("\"\" ") ;
	}
    }

extern ZSC arg_read_int(int *p,char *value)
{
    int r ;
    char *e ;
    r = strtol(value,&e,10) ;
    if (*e) return(ZS_FAIL) ;
    *p = r ;
    return(ZS_OK) ;
    }

extern int arg_read_int_or_die(char *name,char *value)
{
    int r ;
    if (arg_read_int(&r,value) == ZS_OK)
	return(r) ;
    errorfatal("argument '%s' value '%s' must be an integer\n",name,value) ;
    return(0) ;
    }

extern int arg_compress(int argc,char **argv)
{
    char	**a = argv ;
    int		i ;
    int		ndel ;
    for (i = 0 ; i < argc ; i++) {
	if (*argv)
	    *(a++) = *argv ;
	argv++ ;
	}
    ndel = (argv - a) ;
    for (i = ndel ; i > 0 ; ) a[--i] = 0 ;
    return(argc - ndel) ;
    }

extern int arg_read_simple(int argc,char **argv,
			    int (*lfun)(char *,char *,void *),
			    void *a0)
{
    int		i ;
    for (i = 1 ; i < argc ; i++) {
	char *arg = argv[i] ;
	if (!arg) continue ;		/* previously killed */
	if (*arg == '-') {
	    switch (*(arg+1)) {
	    case '-':
		arg_long(argc-i,argv+i,lfun,a0) ;
		break ;
	    default:
		break ;
	    }
	    }
	}
    argc = arg_compress(argc,argv) ;
    return(argc) ;
    }

extern char *arg_get_submode(int argc,char **argv)
{
    int		i ;
    char	*smode ;
    for (i = 1 ; i < argc ; i++) {
	char *arg = argv[i] ;
	if (!arg) continue ;		/* previously killed */
	if (*arg == '-') {
	    switch (*(arg+1)) {
	    case 's':
		smode = (arg+2) ;
		argv[i] = 0 ;
		return(smode) ;
		break ;
	    default:
		break ;
	    }
	    }
	}
    return(0) ;
    }

/* ================================================================ */
static int as_arg_long(int argc,char **argv,ARGSET **as,ARGSET **as_end)
{
    char *arg = (char *) alloca(strlen(*argv)+1) ;
    strcpy(arg,*argv) ;
{
    ALI		z[1] ;
    z->ps = arg ;
    ali_unpack(z,argc,argv) ;

{
    int asf ;
    for ( ; as < as_end ; as++) {
	asf = (*as)->omf(*as,ARGM_ARGTRY,(u32) z) ;
	if (asf == ASF_DEFERRED) return asf ;
	if (asf != ASF_ARGIGNORED) {
	    if (*z->pv && (asf & ASF_VALUEIGNORED))
		if (z->n == 2)
		    z->n = 1 ;
		else errorprintf("value '%s' attached to parameter '%s' ignored",z->pv,z->ps) ;
	    while (z->n-- > 0) *argv++ = 0 ;
	    return asf ;
	}
    }
    return(ASF_ARGUNKNOWN) ;
    }
 }
 }
    
extern int argset_try_as(int argc,char **argv,ARGSET **as,ARGSET **as_end,int die)
{
    int		i ;
    int		asf ;
    for (i = 1 ; i < argc ; i++) {
	char *arg = argv[i] ;
	if (!arg) continue ;		/* previously killed */
	if (*arg == '-') {
	    switch (*(arg+1)) {
	    case '-':
		asf = as_arg_long(argc-i,argv+i,as,as_end) ;
		if (die && asf == ASF_ARGUNKNOWN)
		    errorfatal("try %s --help for help\n",argv[0]) ;
		break ;
	    default:
		break ;
	    }
	    }
	}
    argc = arg_compress(argc,argv) ;
    return(argc) ;
    }

extern int argset_try_all(int argc,char **argv)
{
    return argset_try_as(argc,argv,GSV_ARGSET_start(),GSV_ARGSET_end(),1) ;
    }

extern int argset_try_one(int argc,char **argv,ARGSET *s)
{
    ARGSET *as[2] ;
    as[0] = s ;
    as[1] = 0 ;
    return argset_try_as(argc,argv,as,as+1,0) ;
    }

/* ================================================================ */
extern int arg_peek(int argc,char **argv,char *s)
{
    int		i ;
    for (i = 0 ; i < argc ; i++) {
	if (!strcmp(argv[i],s)) return i ;
	}
    return -1 ;
    }

/* ================================================================ */
#include	<pu/exithook.h>
#include	<pu/mt.h>
#include	"npu_util.h"

typedef struct {
  int		argc ;
  char		**argv ;
  int		argcmax ;
  } ARG ;

typedef struct struct_LB LB ;

struct struct_LB {		/* simplest kind of linked list */
    LB		*next ;
    char	c[1] ;
    } ;

#define LBSIZE(n)	((n) + sizeof(LB) - 1)

static struct {
    ARG 	arg[1] ;
    LB		*lb ;		/* list of inserted arg strings */
    } g ;

static void aadd(ARG *a,char *s)
{
    if (a->argc == a->argcmax) {
	int	n = a->argcmax ? a->argcmax * 4 : 64 ;
	a->argv = (char **) realloc(a->argv,n * sizeof(char *)) ;
	a->argcmax = n ;
	}
    a->argv[a->argc++] = s ;
    }

static char *aalloc(int n)
{
    LB		*r = (LB *) malloc(LBSIZE(n)) ;
    r->next = g.lb ;
    g.lb = r ;
    return r->c ;
    }

static void parse(ARG *a,MT *mt)
{
    char *as,*e,*ae,*aa ;
  
    a->argc = 0 ;
    a->argv = 0 ;
    a->argcmax = 0 ;
    e = mt->s ;

    aa = aalloc(MTFillSize(mt)) ;	/* can't overflow */
    as = ae = aa ;

    while (*e) {
	while (*e == ' ' || *e == '\t' || *e == '\n' || *e == '\r') e++ ;
	if (!*e) goto done ;
	if (*e == '#') {
	    while (*++e != '\n') if (!*e) goto done ;
	    continue ;
	    }
	if (*e == '"' || *e == '\'') {
	    char t = *e ;
	    e++ ;
	    while (1) {
		if (!*e) goto done ;
		else if (*e == t) {
		    *ae++ = 0 ;
		    aadd(a,as) ;
		    as = ae ;
		    break ;
		}
		else if (*e == '\\') {
		    e++ ;
		    if (!*e) {
			errorshow("missing closing quote [%s]",t) ;
			goto done ;
		    }
		}
		*ae++ = *e++ ;
	    }
	}
	else {
	    *ae++ = *e++ ;
	    while (*e && !(*e == ' ' || *e == '\t' || *e == '\n' || *e == '\r')) {
		if (*e == '\'') {if (!*++e) break ;}
		*ae++ = *e++ ;
	    }
	    *ae++ = 0 ;
	    aadd(a,as) ;
	    as = ae ;
	}
    }
done:
    return ;
    }
	
static int arg_inject_read(char *f,ARG *a,int fatal)
{
    MT	mtf[1] ;
    if (mt_file_contents(mtf,f,0) == -1) {
	if (fatal)
	    errorfatal("can't read arg file '%s'\n",f) ;
	return(0) ;
    }
    parse(a,mtf) ;
    mtfree(mtf) ;
    return(1) ;
    }

static void arg_merge(ARG *a,int argc,char **argv,int im)
{
    char	**aargv = (char **) malloc(sizeof(char *) * (argc + a->argc + 1)) ;
    char	**ap = aargv ;
    int		i ;
    for (i = 0 ; i < im ; i++)
	*ap++ = argv[i] ;
    for (i = 0 ; i < a->argc ; i++)
	*ap++ = a->argv[i] ;
    for (i = im ; i < argc ; i++)
	*ap++ = argv[i] ;
    *ap = 0 ;
    free(a->argv) ;
    a->argv = aargv ;
    a->argc += argc ;
    }

static void arg_expand_cleanup(void)
{
    LB		*lb ;
    free(g.arg->argv) ;
    lb = g.lb ;	
    while (lb) {
	LB *next = lb->next ;
	free(lb) ;
	lb = next ;
	}
    }

static void install_cleanup_exithook(void)
{
    exithook_install((EXITHOOK) arg_expand_cleanup,0) ;
    }

extern int arg_expand_try_file(int *pargc,char ***pargv,char *file)
{
    ARG	a[1] ;
    if (!arg_inject_read(file,a,0)) return 0 ;
{
    char	**argv = *pargv ;
    char	argc = *pargc ;
    if (a->argc) {
	arg_merge(a,argc,argv,argc) ;
	if (g.arg->argv) 
	    free(g.arg->argv) ;
	g.arg[0] = a[0] ;
	install_cleanup_exithook() ;
	*pargv = a->argv ;
	*pargc = a->argc ;
	}
    return(1) ;
    }
    }

extern int arg_expand(int *pargc,char ***pargv)
{
    int		i ;
    char	**argv = *pargv ;
    char	argc = *pargc ;
    for (i = 1 ; i < argc ; i++) {
	char *arg = argv[i] ;
	if (!arg) continue ;		/* previously killed */
	if (*arg == '-') {
	    switch (*(arg+1)) {
	    case 'd':
		if (chdir(arg+2) == -1) {
		    errorfatal("failed to chdir to %s\n",arg+2) ;
		    }
		argv[i] = 0 ;
		break ;
	    case 'y': {
		ARG	a[1] ;
		arg_inject_read(arg+2,a,1) ;
		if (a->argc) {
		    arg_merge(a,argc,argv,i+1) ;
		    argv = a->argv ;
		    argc = a->argc ;
		    if (g.arg->argv) free(g.arg->argv) ;
		    g.arg[0] = a[0] ;
		    install_cleanup_exithook() ;
		    }
		argv[i] = 0 ;
		break ;
		}
	    default:
		break ;
	    }
	    }
	}
    argc = arg_compress(argc,argv) ;

    *pargv = argv ;
    *pargc = argc ;

    return(RC_OK) ;
    }

extern ARGITEM *argitem_find(ARGITEM **ii,char *name)
{
    ARGITEM *i ;
    for ( ; i = *ii ; ii++)
	if (!i->matcher(name,i->argname)) return(i) ; 
    return 0 ;
    }

extern int argitem_accept(ARGITEM *i,char *name,char *value,void *a0)
{
    int		asf ;
    if (i->type == AIT_BOOL) {
	if (!*value) value = "1" ;
	else if (!stricmp(value,"no")) value = "0" ;
	else if (!stricmp(value,"off")) value = "0" ;
	else if (!stricmp(value,"false")) value = "0" ;
	else if (!stricmp(value,"on")) value = "1" ;
	else if (!stricmp(value,"true")) value = "1" ;
	else if (!stricmp(value,"yes")) value = "1" ;
	}
    asf = i->accept(name,value,a0) ;
    return asf ;
    }

extern int argitem_try(ARGITEM **ii,char *name,char *value,void *a0)
{
    int		asf ;
    ARGITEM *i = argitem_find(ii,name) ;
    if (!i) {
	if (memcmp_icase(name,"no-",3))	return(ASF_ARGIGNORED) ;
	if (!(i = argitem_find(ii,name+3))) return(ASF_ARGIGNORED) ;
	if (i->type == AIT_BOOL) {
	    if (*value) 
		errorshow("%s=%s not allowed value with 'no-' prefix",name,value) ;
	    value = "0" ;
	}
	else {
	    return(ASF_ARGIGNORED) ;
	    }
	}
    asf = argitem_accept(i,name,value,a0) ;
    return asf ;
    }

/* ================================================================ */
static void argset_null_om_gethelp(ARGSET *as,MT *mto)
{
    ARGITEM **ii,*i ;
    MT mti[1] ;
    MTALLOCA(mti,1024) ;

    mtprintf(mto,"%s\n",as->help) ;
    for (ii = as->items ; i = *ii ; ii++) {
	MTREWIND(mti) ;
	mtputs(mti,i->argname) ;
	if (i->flags & ASF_ARGNAMESHORT) {
	    if (*i->valuename) 
		mtprintf(mti,"%s",i->valuename) ;
	    mtprintf(mti," (%s)",names_ait_lc[i->type]) ;
	    mtprintf(mto,"   -%-40s%s\n",mti->s,i->help) ;
	}
	else {
	    if (*i->valuename) 
		mtprintf(mti,"=%s",i->valuename) ;
	    mtprintf(mti," (%s)",names_ait_lc[i->type]) ;
	    mtprintf(mto,"  --%-40s%s\n",mti->s,i->help) ;
	}
    }
    mtprintf(mto,"\n") ;
}

static int generic_argtry(ARGSET *o,ALI *a)
{
    ARGITEM *i = argitem_find(o->items,a->ps) ;
    if (!i) return ASF_ARGIGNORED ;
    return(i->accept(a->ps,a->pv,a)) ;
    }

extern u32 argset_omf_null(ARGSET *o,int m,u32 a)
{
    switch(m) {
	case OM_GETHELP:	argset_null_om_gethelp(o,(MT *) a) ; return(0) ;
	case ARGM_ARGTRY:	return(generic_argtry(o,(ALI *) a)) ;
	}
    return(0) ;
    }

extern u32 argitem_omf_null(ARGITEM *o,int m,u32 a)
{
    switch(m) {
	case OM_GETHELP:	mtputs((MT *) a,o->help) ; return(0) ;
	}
    return(0) ;
    }

#include	".gen/arg.c"

#ifdef __cplusplus /*Z*/
}
#endif
