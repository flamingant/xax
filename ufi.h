#ifdef __cplusplus /*Z*/
extern "C" {
#endif

#ifndef __ufi_h
#define __ufi_h

#ifndef __stimer_h
#include	"stimer.h"
#endif

#ifndef __iterator_h
#include	<pu/iterator.h>
#endif

typedef struct struct_UFIC UFIC ;

typedef u32 (*UFIOMA)(UFIC *,int,u32) ;

struct struct_UFIC {
    void	*cc ;
    ITERATOR	it[1] ;
    STIMER	*st ;
    void	*next ;
    struct {
	int	before ;
	int	between ;
	} wait ;
    UFIOMA	oma ;
    /* copied from iterator data for diagnostic purposes */
    int		iti ;
    int		itn ;
    } ;

enum UFIM {
    UFIM_START,
    UFIM_FIRST,
    UFIM_NEXT,
    UFIM_LAST,
    UFIM_FINISH,
    UFIM_ABORT,
    UFIM__USER,
    } ;

extern void ufi_start(UFIC *c) ;
extern void ufi_abort(UFIC *c,int) ;

#endif

#ifdef __cplusplus /*Z*/
}
#endif
