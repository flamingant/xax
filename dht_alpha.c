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
#include	"tc_sql.h"

#include	"common.h"

/* ================================================================ */
/* ================================================================ */
/* this can only be done after we have the name of the torrent etc */

extern void tanno_parse_torrent_name(TANNO *t,char *name) ;

static void torrent_alphabetize_response(UF *uf,HT *ht,RBP *r)
{
    }

static u32 torrent_alphabetize_uff(UF *uf,int m,u32 a)
{
    HT		*ht = uf->d.ht ;
    switch(m) {
    case UFM_TYPENAME:	return(mtputs((MT *) a,"ALPHABETIZE")) ;
    case UFM_HT_RESPONSE_JSON:
	torrent_alphabetize_response(uf,ht,(RBP *) a) ;
	break ;
    default:
	break ;
	}
    return(dht_uff(uf,m,a)) ;
    }

#include	<pu/xctype.h>

extern void path_sanitize(uchar *s)
{
    uchar *d = s ;
    while (*s) {
	if (*s < 0x80 && *s > 0x1f && !strchr("\\/?*:<>|",*s))
	    *d++ = *s ;
	s++ ;
	}
    *d = 0 ;
    }

extern void tanno_alphabetize(MT *mtr,TANNO *t)
{
    MT		mt[1] ;
    char	*artist ;
    char	*title ;
    char	init ;
    MTALLOCA(mt,1024) ;
    artist = mtcaches(mt,t->artist) ;
    title = mtcaches(mt,t->title) ;
    path_sanitize(artist) ;
    path_sanitize(title) ;
    init = artist[0] ;
    if (init >= 'a' && init <= 'z') init &= 0xdf ;
    mtprintf(mtr,"%c/%s/%s",artist[0],artist,title) ;
    }

static void torrent_alphabetize_qb_callback(TANNO *t,TOD *tod)
{
    MT		mt[1] ;
    char	*artist ;
    char	*title ;
    char	*path ;
    char	init ;
    MTALLOCA(mt,1024) ;
    artist = mtcaches(mt,t->artist) ;
    title = mtcaches(mt,t->title) ;
    path_sanitize(artist) ;
    path_sanitize(title) ;
    init = artist[0] ;
    if (init >= 'a' && init <= 'z') init &= 0xdf ;
    path = mt->c ;
    mtprintf(mt,"%c/%s/%s",artist[0],artist,title) ;
    printf("%s - %s ---> %s\n",t->artist,t->title,path) ;
    fflush(stdout) ;
    /* this code is incomplete */
    /* set save path */
    }

/* leaving out units with features we don't want only works if they
   are only invoked on demand, either from a command line
   or web interface. If they are explicitly called (as previously
   in the case of torrent_alphabetize) omitting them from the link
   will cause an undefined symbol
   */

extern void torrent_alphabetize(TOD *tod)
{
    TANNO	t[1] ;
    t->tid = tod->tid ;
    tanno_query(t,(void (*)(TANNO *,u32))torrent_alphabetize_qb_callback,(u32) tod) ;
    }

/* ================================================================ */
#include	"http.h"

/* ~# use http ; #~ */

typedef struct {
    char	*error ;
    char	*hash ;
   } TA_ALPHABETIZE_C ;

/* ~~hpf()~~ */
static u32 hpf_torrent_alphabetize(HSR *r,int m,u32 a)
{
    TA_ALPHABETIZE_C	*c = r->lc ;
    switch (m) {
    case HPM_TORRENT_APPLY_OK:
	return(1) ;
    case HPM_DESTROY:
	free(c) ;
	break ;
    case HPM_CREATE:
	r->lc = c = calloc(1,sizeof(TA_ALPHABETIZE_C)) ;
	if (!(c->hash = hsr_arg_exists(r,"hash"))) {
	    c->error = "No torrent hash supplied" ;
	    return(0) ;
	    }
	break ;
    case HPM_INTERNAL: {
	TOD *tod ;
	tod = tod_intern(c->hash) ;
	torrent_alphabetize(tod) ;
	return(0) ;
	}
    case HPM_CONTENT_SEND:
{
    hsr_printf(r,"<h2>Alphabetized torrent [%s]</h2>",c->hash) ;
    return(0) ;
    }
    default:
	break ;
	}
    return(hpf_generic_ok(r,m,a)) ;
    }

#include	".gen/dht_alpha.c"

#ifdef __cplusplus /*Z*/
}
#endif
