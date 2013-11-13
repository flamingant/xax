#ifdef __cplusplus /*Z*/
extern "C" {
#endif

#include	<unistd.h>
#include	<string.h>

#include	"pu/mt.h"

#include	"pu/rcons.h"

#include	"log.h"

#include	"ma.h"
#include	"dht.h"

#include	"jsf.h"
#include	"uf.h"

#include	"http.h"

#include	"common.h"

/* ~# use http ; #~ */

/* ================================================================ */
static u32 torrent_free_move_data(void)
{
    return(0) ;
    }

/* ~~hpf()~~ */
extern u32 hpf_torrent_free_move_data(HSR *r,int m,u32 a)
{
    switch (m) {
    case HPM_INTERNAL:
	return(torrent_free_move_data()) ;
    case HPM_CONTENT_SEND:
	hsr_printf(r,"Done") ;
	return(0) ;
    default:
	break ;
	}
    return(hpf_generic_ok(r,m,a)) ;
    }

/* http://deluge-torrent.org/docs/master/modules/ui/web/json_api.html */
#include	".gen/dht_move.c"

#ifdef __cplusplus /*Z*/
}
#endif
