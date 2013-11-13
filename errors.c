#ifdef __cplusplus /*Z*/
extern "C" {
#endif

#include	<stdio.h>
#include	<stdarg.h>
#include	<stdlib.h>
#include	<string.h>

#include	"errors.h"

/* ================================================================ */
typedef struct {
    char	*s ;	/* start */
    int		size ;	/* size */
} MS ;

#define MSSET(M,S,N)	{(M)->s = S ; (M)->size = N ;}
#define MSSize(M)	((M)->size)
#define MSEnd(M)	((M)->s + (M)->size)

#define MSALLOCA(M,N) 	MSSET(M,alloca(N),N)

/* ================================================================ */
/* 
   a function which sets the error string should not call a function which
   sets the error string, but if it does, it should create a new error
   string which includes the previous string
   */

struct {
    char	*last ;
    MS		m[1] ;
    FILE	*fh ;
    struct {
	void (*fun)(void *,int) ;
	void *arg ;
    } fatal ;
    } error = {""} ;

extern void errorset_fh(FILE *file)
{
    error.fh = file ;
    }

extern char *errorsetlast(char *msg)
{
    int len = strlen(msg) + 1 ;
    if (len > MSSize(error.m)) {
	if (error.m->s) {
	    error.m->s = (char *) realloc(error.m->s,len) ;
	    }
	else {
	    error.m->s = (char *) malloc(len) ;
	    }
	error.m->size = len ;
	}
    strcpy(error.m->s,msg) ;
    error.last = error.m->s ;
    return error.m->s ;
    }

extern char *errorset_va(char *fmt,va_list va)
{
    char	msg[1024] ;
    vsnprintf(msg,sizeof(msg),fmt,va) ;
    return errorsetlast(msg) ;
    }

extern char *errorset(char *fmt,...)
{
    va_list	va ;
    va_start(va,fmt) ;
    return errorset_va(fmt,va) ;
    }

/* ================================================================ */
extern void errorfatal_register(void (*fun)(void *,int),void *arg)
{
    error.fatal.arg = arg ;
    error.fatal.fun = fun ;
    }

extern void errorfatal_exit(int code)
{
    if (error.fatal.fun)
	error.fatal.fun(error.fatal.arg,code) ;
    else exit(code) ;
    }

/* ================================================================ */
RCONS *error_callbacks ;

static void error_callback(char *s)
{
    RCONS *r ;
    for (r = error_callbacks ; r ; r = r->cdr) {
	void (*fun)(char *) = (void (*)(char *)) r->car ;
	fun(s) ;
	}
    }

extern void error_callback_register(void (*fun)(char *))
{
    }

extern void error_callback_unregister(void (*fun)(char *))
{
    }

/* ================================================================ */
extern void errorputs(char *s)
{
    int len = strlen(s) ;
    FILE	*fh ;
    fh = error.fh ? error.fh : stdout ;
    error_callback(s) ;
    fputs(s,fh) ;
    if (len > 0 && s[len-1] != '\n') fputs("\n",fh) ;
    fflush(fh) ;
    }

extern void errorshow_va(char *fmt,va_list va)
{
    errorset_va(fmt,va) ;
    errorputs(error.last) ;
    }

extern void errorshow(char *fmt,...)
{
    va_list	va ;
    va_start(va,fmt) ;
    errorshow_va(fmt,va) ;
    }

extern void errorprintf_va(char *fmt,va_list va)
{
    char	msg[1024] ;
    vsnprintf(msg,sizeof(msg),fmt,va) ;
    errorputs(msg) ;
    }

extern void errorprintf(char *fmt,...)
{
    va_list	va ;
    va_start(va,fmt) ;
    errorprintf_va(fmt,va) ;
    }

extern void errorfatal(char *fmt,...)
{
    va_list	va ;
    va_start(va,fmt) ;
    errorset_va(fmt,va) ;
    errorputs(error.last) ;
    errorfatal_exit(1) ;
    }

extern void errorclear()
{
    if (error.m->s) *error.m->s = 0 ;
    }

extern int assertzerofatal(void *p)
{
    if (p) {
	errorprintf("fatal - non-zero assertion failure\nlast.error = \"%s\"\n",error.last) ;
	errorfatal_exit(1) ;
	return 0 ;
	}
    return 1 ;
    }

extern int assertzerofatals(void *p,char *s)
{
    if (p) {
	errorprintf("fatal - non-zero assertion failure (%s)\nlast.error = \"%s\"\n",s,error.last) ;
	errorfatal_exit(1) ;
	return 0 ;
	}
    return 1 ;
    }

extern int assertzero(void *p)
{
    if (p) {
	errorprintf("warning - non-zero assertion failure\nlast.error = \"%s\"\n",error.last) ;
	return 0 ;
	}
    return 1 ;
    }

extern int assertzeros(void *p,char *s)
{
    if (p) {
	errorprintf("warning - non-zero assertion failure (%s)\nlast.error = \"%s\"\n",s,error.last) ;
	return 0 ;
	}
    return 1 ;
    }

extern int assertnonzerofatal(void *p)
{
    if (!p) {
	errorprintf("fatal - non-zero assertion failure\nlast.error = \"%s\"\n",error.last) ;
	errorfatal_exit(1) ;
	return 0 ;
	}
    return 1 ;
    }

extern int assertnonzerofatals(void *p,char *s)
{
    if (!p) {
	errorprintf("fatal - non-zero assertion failure (%s)\nlast.error = \"%s\"\n",s,error.last) ;
	errorfatal_exit(1) ;
	return 0 ;
	}
    return 1 ;
    }

extern int assertnonzero(void *p)
{
    if (!p) {
	errorprintf("warning - non-zero assertion failure\nlast.error = \"%s\"\n",error.last) ;
	return 0 ;
	}
    return 1 ;
    }

extern int assertnonzeros(void *p,char *s)
{
    if (!p) {
	errorprintf("warning - non-zero assertion failure (%s)\nlast.error = \"%s\"\n",s,error.last) ;
	return 0 ;
	}
    return 1 ;
    }

extern void errorshow_and(int what,char *fmt,...)
{
    va_list	va ;
    if (what == OR_NOTHING) return ;
    va_start(va,fmt) ;
    errorshow_va(fmt,va) ;
    if (what == OR_DIE) exit(1) ;
    }

extern void unimplemented(const char *s)
{
    errorshow("%s is unimplemented",s) ;
    }

/* ================================================================ */
extern FILE *fopen_and_check(char *name,char *mode)
{
    FILE *f ;
    if ((f = fopen(name,mode)) == NULL) {
	errorshow("failed to open file %s in mode '%s'",name,mode) ;
	}
    return(f) ;
    }


#ifdef __cplusplus /*Z*/
}
#endif
