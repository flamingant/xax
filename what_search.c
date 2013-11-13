#ifdef __cplusplus /*Z*/
extern "C" {
#endif

#include	<string.h>

#include	"what.h"
#include	"common.h"

#include	".gen/what_search.h"
/* ================================================================ */
typedef struct {
    HSR		*hsr ;
    UF 		*ufparent ;
    UF 		*ufchild ;
    json_t	*root ;
    UFIOMA	sub ;

    RTREE	*groups ;

    } WHAT_SEARCH_UFICC ;

static u32 what_search_uff(UF *uf,int m,u32 a) ;

static u32 what_search_add_ufic_oma(UFIC *c,int m,u32 a) ;
static u32 what_search_foo_ufic_oma(UFIC *c,int m,u32 a) ;

static UFIOMA sub_lookup(HSR *r)
{
    GLT *t = glrassoc(glt_glub,hsr_arg_or_default(r,"sub","foo"),(GLTCMP) strcmp) ;
    if (t) return (UFIOMA) t->car ;
    return what_search_foo_ufic_oma ;
    }

extern void what_search(HSR *r)
{
    HT		*ht ;
    UF		*uf ;
    MT		mt[1] ;
    WHAT_SEARCH_UFICC	*cc = tscalloc(1,WHAT_SEARCH_UFICC) ;
    MTALLOCA(mt,4096) ;
    mtputs(mt,"ajax.php?action=browse") ;
    mtprintf(mt,"&freetorrent=1") ;
    mtprintf(mt,"&page=%s",hsr_arg_or_default(r,"page","1")) ;
    ht = https_create("GET","what.cd",mt->s,0) ;
    cc->sub = sub_lookup(r) ;
    cc->hsr = r ;
    ht->cookie = what_g.cookie ;
    uf = uf_create(what_search_uff,ht,cc) ;
    ufs_rc_add(uf) ;
    }

enum WHATUFIM {
    UFIM_TORRENT = UFIM__USER,
    } ;

typedef struct {
    json_t	*e  ;
    json_t	*jid  ;
    int		id ;
    } UFIM_TORRENT_ARG ;

/* ================================================================ */
/* ~# use decode ; #~ */
/* ~~nametable("^static u32 (what_search_(\\w+)_ufic_oma).*\\)\\s*\$",
   tag => "glub",
   transform => '"$2"',
   format => 'glt',
   )~~ */

static u32 what_search_add_ufic_oma(UFIC *c,int m,u32 a)
{
    WHAT_SEARCH_UFICC *cc = (typeof(cc)) c->cc ;
    switch(m) {
    case UFIM_TORRENT: {
	UFIM_TORRENT_ARG *ta = (typeof(ta)) a ;
	if (torrent_id_to_hash(ta->id,0) == 0) {
	    log_printf_c(ls_what," adding\n") ;
	    cc->ufchild = dht_add_torrent_url_i(ta->id) ;
	    cc->ufchild->parent = cc->ufparent ;
	    stimer_pause(c->st) ;
	    }
	else {
	    log_printf_c(ls_what," skipped\n") ;
	    stimer_retrigger(c->st) ;
	    }
	}
    }
    return(0) ;
    }

static u32 what_search_foo_ufic_oma(UFIC *c,int m,u32 a)
{
    WHAT_SEARCH_UFICC *cc = (typeof(cc)) c->cc ;
    (void) cc ;
    switch(m) {
    case UFIM_TORRENT: {
	UFIM_TORRENT_ARG *ta = (typeof(ta)) a ;
	if (torrent_id_to_hash(ta->id,0) == 0) {
	    log_printf_c(ls_what," FOO %d no\n",ta->id) ;
	    }
	else {
	    log_printf_c(ls_what," FOO %d yes\n",ta->id) ;
	    }
	stimer_retrigger(c->st) ;
	}
    }
    return(0) ;
    }

static u32 what_search_group_ufic_oma(UFIC *c,int m,u32 a)
{
    WHAT_SEARCH_UFICC *cc = (typeof(cc)) c->cc ;
    (void) cc ;
    switch(m) {
    case UFIM_TORRENT: {
	UFIM_TORRENT_ARG *ta = (typeof(ta)) a ;
	int gid = json_integer_value(json_object_get(ta->e,"groupId")) ;
	RTREE **r = rtree_find_(&cc->groups,(RKEY) gid,0) ;
	log_printf_c(ls_what," %d group id %d %s\n",ta->id,gid,*r ? "+" : "-") ;
	if (!*r) {
	    hsr_printf(cc->hsr,"<br><a href=\"/what?cmd=group&group=%d\">%d</a>",gid,gid) ;
	    *r = rtree_create((RKEY) gid,0,0,0) ;
	}
	stimer_retrigger(c->st) ;
	}
    }
    return(0) ;
    }

/* ================================================================ */
static u32 what_search_ufic_oma(UFIC *c,int m,u32 a)
{
    WHAT_SEARCH_UFICC *cc = (typeof(cc)) c->cc ;
    switch(m) {
    case UFIM_START:
 	log_printf(ls_what,"start count = %d\n",GDUCAST(a)->i) ;
	break ;
    case UFIM_FIRST:
	break ;
    case UFIM_NEXT: {
	UFIM_TORRENT_ARG ta[1] ;
	ta->e = (json_t *) c->next ;
	ta->jid = json_object_get(ta->e,"torrentId") ;
	ta->id = json_integer_value(ta->jid) ;
	log_printf(ls_what,"next (%d of %d) %d",c->iti,c->itn,ta->id) ;
	cc->sub(c,UFIM_TORRENT,(u32) ta) ;
	break ;
	}
    case UFIM_LAST:
	break ;
    case UFIM_FINISH:
	iterator_close(c->it) ;
	json_iterator_destroy(c->it) ;
	hsr_close(cc->hsr) ;
	uf_destroy(cc->ufparent) ;
	log_printf(ls_what,"done\n") ;
	break ;
    }
    fflush(stdout) ;
    return(0) ;
    }

/* ================================================================ */
static u32 what_search_parent_uff(UF *uf,int m,u32 a)
{
    UFIC *c = (typeof(c)) uf->d.v ;
    WHAT_SEARCH_UFICC *cc ;
    switch(m) {
    case UFM_TYPENAME:
	return(mtputs((MT *) a,"WHAT_SEARCH")) ;
    case UFM_DESTROY: {
	cc = (typeof(cc)) c->cc ;
	json_decref(cc->root) ;
	free(c->cc) ;
	free(c) ;
	return 0 ;
	}
    case UFM_PRE_NOTIFY: {
	UFM_NOTIFY_ARG	*na = (typeof(na)) a ;
	switch (na->m) {
	case UFM_DESTROY:
	    stimer_unpause(c->st) ;
	    break ;
	    }
	return 0 ;
	}
    case UFM_POST_NOTIFY: {
	cc = (typeof(cc)) c->cc ;
	UFM_NOTIFY_ARG	*na = (typeof(na)) a ;
	switch (na->m) {
	case UFM_TORRENT_ADDED: {
	    TOD		*tod = (typeof(tod)) na->a ;
	    torrent_label(tod,"free") ;
	    return 1 ;
	    }
	}
	return 0 ;
	}
    }
    return(null_uff(uf,m,a)) ;
    }

extern void what_search_accept_response(UF *uf,RBP *r)
{
    HT		*ht = uf->d.ht ;
    json_t *ej = json_object_get(r->j,"status") ;
    json_t *rj = json_object_get(r->j,"response") ;
    json_t *results = json_object_get(rj,"results") ;
    int		npage ;
    int		ipage ;
    ipage = json_integer_value(json_object_get(rj,"currentPage")) ;
    npage = json_integer_value(json_object_get(rj,"pages")) ;
    const char *s = json_string_value(ej) ;
    (void) s ;
//    json_dump_stdout(results) ;
{
    WHAT_SEARCH_UFICC *cc = (typeof(cc)) ht->sc ;
    UFIC *c = tscalloc(1,UFIC) ;
    c->cc = cc ;
    cc->root = json_deep_copy(results) ;
    cc->ufparent = uf_create(what_search_parent_uff,c,0) ;
    json_array_iterator_open(c->it,cc->root,0) ;
    c->wait.before = 2000 ;
    c->wait.between = 1000 ;
    c->oma = what_search_ufic_oma ;
    ufi_start(c) ;
}
}

static u32 what_search_uff(UF *uf,int m,u32 a)
{
    switch(m) {
    case UFM_DESTROY:				/*A*/
	break ;
    case UFM_TYPENAME:	return(mtputs((MT *) a,"what.search")) ;
    case UFM_HT_RESPONSE_JSON: {
	what_search_accept_response(uf,((RBP *) a)) ;
	break ;
	}
    default:
	break ;
	}
    return(wht_uff(uf,m,a)) ;
    }

/*
https://what.cd/torrents.php?
freetorrent=1
&tags_type=1
&order_by=time
&order_way=desc
&action=advanced
&searchsubmit=1
*/

#include	".gen/what_search.c"

#ifdef __cplusplus /*Z*/
}
#endif
