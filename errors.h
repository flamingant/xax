#ifdef __cplusplus /*Z*/
extern "C" {
#endif

#ifndef __errors_h
#define __errors_h

#ifdef uchar
#undef uchar
#endif

#define RC_OK		0
#define RC_ERROR	-1
#define RC_FATAL	-999

typedef enum {
    ZS_OK	= 0,
    ZS_FAIL	= 1,
    } ZSC ;

#define OR_NOTHING	0
#define OR_WARN		1
#define OR_DIE		2

extern void errorfatal_register(void (*fun)(void *,int),void *arg) ;

#ifndef __rcons_h
#include	<pu/rcons.h>
#endif

extern RCONS *error_callbacks ;

extern void errorputs(char *s) ; 
extern void errorprintf(char *fmt,...) ; 

extern void errorshow_and(int,char *fmt,...) ;

extern char *errorsetlast(char *msg) ; 
extern char *errorset(char *fmt,...) ;
extern void errorshow(char *fmt,...) ;
extern void errorfatal(char *fmt,...) ;
extern void errorclear() ;

/* all return 0 if the asserion fails */

extern int assertnonzerofatal(void *p) ;
extern int assertnonzerofatals(void *p,char *s) ;
extern int assertnonzero(void *p) ;
extern int assertnonzeros(void *p,char *s) ;

extern int assertzerofatal(void *p) ;
extern int assertzerofatals(void *p,char *s) ;
extern int assertzero(void *p) ;
extern int assertzeros(void *p,char *s) ;

#define STRING2(x) #x
#define STRING(x) STRING2(x)
#define SRCREF(s) __FILE__ ":" STRING(__LINE__) ": " s

#define AZF(s) assertzerofatals((void *) (s),__FILE__ ":" STRING(__LINE__) ": AZF(" #s ")")
#define AZW(s) assertzeros((void *) (s),__FILE__ ":" STRING(__LINE__) ": AZW(" #s ")")
#define ANZF(s) assertnonzerofatals((void *) (s),__FILE__ ":" STRING(__LINE__) ": ANZF(" #s ")")
#define ANZW(s) assertnonzeros((void *) (s),__FILE__ ":" STRING(__LINE__) ": ANZW(" #s ")")

#define ASSERTFATAL(s) assertnonzerofatals((void *) (s),__FILE__ ":" STRING(__LINE__) ": ASSERTFATAL(" #s ")")
#define ASSERTWARN(s) assertnonzeros((void *) (s),__FILE__ ":" STRING(__LINE__) ": ASSERTWARN(" #s ")")

#define HERE(x)	{printf(__FILE__ ":%d:%s:%s\n",__LINE__,__FUNCTION__,x) ; fflush(stdout) ;}
#define CHERE(x) {cprintf(__FILE__ ":%d:%s:%s\n",__LINE__,__FUNCTION__,x) ;}

#ifdef NEED_ERRORS_STDIO
#include	<stdio.h>

extern FILE *fopen_and_check(char *name,char *mode) ;

extern void errorset_fh(FILE *file) ;

#endif

#endif

#ifdef __cplusplus /*Z*/
}
#endif
