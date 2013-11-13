#ifdef __cplusplus /*Z*/
extern "C" {
#endif

#include	"atinit.h"

extern ATINITFUN atinit_initvec[] ;

extern u32 atinit(int phase,u32 a)
{
    int		r = 0 ;
    ATINITFUN *fun = atinit_initvec ;
    while (*fun) {
	r |= (*fun)(phase,a) ;
	fun++ ;
	}
    return r ;
    }

extern u32 atinit_premode(int argc,char **argv,char *mode)
{
    ATINIT_MAIN_ARG	a[1] ;
    a->argc = argc ;
    a->argv = argv ;
    a->mode = mode ;
    return(atinit(AT_MAIN_PRE_MODE,(u32) a)) ;
    }


#ifdef __cplusplus /*Z*/
}
#endif
