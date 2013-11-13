#ifdef __cplusplus /*Z*/
extern "C" {
#endif

#ifndef __atinit_h
#define __atinit_h	1

typedef struct {
    int		argc ;
    char	**argv ;
    char	*mode ;
    } ATINIT_MAIN_ARG ;

#define AT_MAIN_PRE_MODE	0

#ifndef __type_h
#include	<pu/type.h>
#endif

typedef u32 (*ATINITFUN)(int,u32) ;

extern u32 atinit(int phase,u32 a) ;
extern u32 atinit_premode(int argc,char **argv,char *mode) ;

#endif

#ifdef __cplusplus /*Z*/
}
#endif
