#ifdef __cplusplus /*Z*/
extern "C" {
#endif

#include	<malloc.h>
#include	<pu/hash.h>

#include	"hashtab.h"

extern HTC *htc_alloc(void)
{
    return((HTC *) malloc(sizeof(HTC))) ;
    }

extern HTC *htc_find(HASHTAB *tab,char *pb,int cb,HTC ***rslot)
{
    HTC		*i ;
    long	hash = hashstring_generic(pb,cb) ;
    HTC		**slot ;
    slot = tab->slots.p + hash % tab->slots.n ;
    if (rslot) *rslot = slot ;
    for (i = *slot ; i != EMPTYSLOT ; ) {
	if (!tab->keycmp(i->d,pb,cb)) return(i) ;
	i = i->next ;
	}
    return(EMPTYSLOT) ;
    }

extern void tab_slot_inits(HASHTAB *tab,int n)
{
    tab->slots.n = n ;
    tab->slots.p = (typeof(tab->slots.p)) calloc(n,sizeof(HTC *)) ;
    }

extern void hashtab_map(HASHTAB *tab,void (*fun)(HTD *,u32),u32 a)
{
    int		is ;
    for (is = 0 ; is < tab->slots.n ; is++) {
	HTC **slot = tab->slots.p + is ;
	HTC *s ;
	for (s = *slot ; s != EMPTYSLOT ; ) {
	    fun(s->d,a) ;
	    s = s->next ;
	    }
	}
    }

extern void hashtab_destroy(HASHTAB *tab)
{
    int		is ;
    for (is = 0 ; is < tab->slots.n ; is++) {
	HTC **slot = tab->slots.p + is ;
	HTC *s,*next ;
	for (s = *slot ; s != EMPTYSLOT ; s = next) {
	    next = s->next ;
	    free(s) ;
	    }
	}
    free(tab->slots.p) ;
    }

/* ================================================================ */

#ifdef __cplusplus /*Z*/
}
#endif
