#ifdef __cplusplus /*Z*/
extern "C" {
#endif

#ifndef __hashtab_h
#define __hashtab_h	1

#include	<pu/type.h>

typedef struct struct_htc HTC ;

#ifdef HTD_TYPE
typedef HTD_TYPE HTD ;
#else
typedef char HTD ;
#endif

struct struct_htc {
    HTD		*d ;
    HTC		*next ;
    }  ;

typedef struct {
    int		(*keycmp)(HTD *,char *,int) ;
    struct {
	HTC	**p ;
	int	n ;
	} slots ;
    } HASHTAB ;

#define EMPTYSLOT	0

extern HTC *htc_find(HASHTAB *tab,char *pb,int cb,HTC ***rslot) ;
extern void hashtab_map(HASHTAB *tab,void (*fun)(HTD *,u32),u32 a) ;
extern void hashtab_destroy(HASHTAB *tab) ;
extern void tab_slot_inits(HASHTAB *tab,int n) ;
extern HTC *htc_alloc(void) ;

#endif

#ifdef __cplusplus /*Z*/
}
#endif
