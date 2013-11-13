#ifdef __cplusplus /*Z*/
extern "C" {
#endif

#ifndef __tod_h
#define __tod_h

#ifndef __type_h
#include	<pu/type.h>
#endif

typedef i64 int64 ;

typedef struct struct_TSI TSI ;

#include	"sf.h"
#include	".gen/tod.h"

typedef struct {
    struct {
	u8	enable : 1 ;
	} state ;
    int		period ;
    } HCP ;

#ifndef __stimer_h
#include	"stimer.h"
#endif

typedef struct {
    char	*hash ;
    int		tid ;
    TSI		*tsi_c ;
    HCP		hcp ;
    STIMER	*st ;
    } TOD ;

extern void tod_tab_map(void (*fun)(TOD *,u32),u32 a) ;

extern TOD *tod_intern(char *hash) ;

extern void tod_tab_init(void) ;
extern void tod_tab_destroy(void) ;

extern void tsi_destroy(TSI *tsi) ;

#ifndef __mt_h
#include	<pu/mt.h>
#endif

extern int tod_identify(MT *mt,TOD *t) ;

#ifdef __log_h
extern void tod_log_printf(LOGSEC *ls,TOD *t,char *fmt,...) ;
#endif

#endif

#ifdef __cplusplus /*Z*/
}
#endif
