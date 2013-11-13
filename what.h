#ifdef __cplusplus /*Z*/
extern "C" {
#endif

#ifndef __what_h
#define __what_h

#include	<pu/rtree.h>
#include	"log.h"
#include	"ma.h"
#include	"dht.h"
#include	"uf.h"
#include	"ufi.h"
#include	"jsf.h"
#include	"http.h"
#include	"tc_sql.h"

typedef struct struct_WSC WSC ;

struct struct_WSC {
    HSR		*hsr ;
    UFIOMA	sub ;
    RTREE	*groups ;
    } ;

extern LOGSEC ls_what[] ;

extern u32 wht_uff(UF *uf,int m,u32 a) ;

typedef struct {
    char	*cookie ;
    } WHAT_G ;

extern WHAT_G what_g ;

#endif

#ifdef __cplusplus /*Z*/
}
#endif
