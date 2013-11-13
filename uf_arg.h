#ifdef __cplusplus /*Z*/
extern "C" {
#endif

#ifndef __uf_arg_h
#define __uf_arg_h

#if defined(UF_ARG_WANT_ALL) || defined(UF_ARG_WANT__UFM_ARG_DESCRIBE)

typedef struct {
    MT		*mt ;
    int		m ;
    u32		a ;
    } UFM_ARG_DESCRIBE_ARG ;

extern int UFM_ARG_DESCRIBE_apply(int (*)(UF *,int,u32,MT *),UF *,u32) ;

#endif

#endif

#ifdef __cplusplus /*Z*/
}
#endif
