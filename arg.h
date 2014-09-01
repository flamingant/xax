#ifdef __cplusplus /*Z*/
extern "C" {
#endif

#ifndef __arg_h
#define __arg_h

#include	"errors.h"

extern int arg_long_or(int what,int argc,char **argv,int (*fun)(char *,char *,void *),void *a0) ;
extern int arg_long(int argc,char **argv,int (*fun)(char *,char *,void *),void *a0) ;

extern void arg_copy(char **from,char *to) ;
extern void arg_dump(int argc, char **argv, int show_missing) ;

extern ZSC arg_read_int(int *p,char *value) ;
extern int arg_read_int_or_die(char *name,char *value) ;

extern int arg_compress(int argc,char **argv) ;

extern int arg_read_simple(int argc,char **argv,
			   int (*lfun)(char *,char *,void *),
			   void *a0) ;

extern char *arg_get_submode(int argc,char **argv) ;

extern int arg_expand(int *pargc,char ***pargv) ;
extern int arg_expand_try_file(int *pargc,char ***pargv,char *file) ;

extern int arg_peek(int argc,char **argv,char *s) ;

/* flags double as return codes */

#define ASF_ARGIGNORED			0
#define ASF_ARGACCEPTED			1
#define ASF_VALUEIGNORED		2
#define ASF_ARGUNKNOWN			4
#define ASF_DEFERRED			8

/* not return codes */

#define ASF_ARGNAMESHORT		0x100

typedef struct __struct_ARGITEM ARGITEM ;
typedef struct __struct_ARGSET ARGSET ;

/* types */

#define AIT_INT		0
#define AIT_BOOL	1
#define AIT_CHAR	2
#define AIT_COMMAND	3

/* ~# use decode ; #~ */
/* ~~define_decode("AIT_(\\w+)",tag => "ait_lc",transform => 'lc($2)')~~ */
/* ~~define_decode("AIT_(\\w+)",tag => "ait_lc",format => 'stringarray',transform => 'lc($2)')~~ */
/* ~~define_decode("AIT_(\\w+)",tag => "ait_uc",transform => 'uc($2)')~~ */

struct __struct_ARGITEM {
    char	*help ;
    char	*argname ;
    char	*valuename ;
    int		(*matcher)(char *,char *) ;
    int		(*accept)(char *,char *,void *) ;
    int		type ;
    int		flags ;
    u32		(*omf)(ARGITEM *,int,u32) ;
    } ;

struct __struct_ARGSET {
    ARGITEM	**items ;
    char	*help ;
    int		flags ;
    u32		(*omf)(ARGSET *,int,u32) ;
    } ;

typedef struct {
    char	*ps ;
    char	*pv ;
    int		n ;
    } ALI ;

extern int ali_as_all_try(ALI *z) ;
extern int ali_as_try(ALI *z,ARGSET **as,ARGSET **as_end) ;

extern ARGITEM *argitem_find(ARGITEM **ii,char *name) ;
extern int argitem_try(ARGITEM **ii,char *name,char *value,void *a0) ;

extern u32 argset_omf_null(ARGSET *,int,u32) ;
extern u32 argitem_omf_null(ARGITEM *,int,u32) ;

#define OM_GETHELP		0
#define ARGM_ARGTRY		1

extern int argset_try_all(int argc,char **argv) ;
extern int argset_try_one(int argc,char **argv,ARGSET *s) ;

#define GSV_ARGSET_ADD(x) \
    ARGSET *gsv_ ## x __attribute__ ((section ("GSV_ARGSET"))) = { x } ;

extern ARGSET **GSV_ARGSET_start(void) ;
extern ARGSET **GSV_ARGSET_end(void) ;

#endif

#ifdef __cplusplus /*Z*/
}
#endif
