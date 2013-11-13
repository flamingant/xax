#ifdef __cplusplus /*Z*/
extern "C" {
#endif

#include	"what.h"
#include	"common.h"

/* ================================================================ */
typedef struct {
    UF 		*ufparent ;
    UF 		*ufchild ;
    json_t	*root ;
    UFIOMA	sub ;
    WSC		*wsc ;
    } WHAT_GROUP_UFICC ;

static u32 what_group_ufic_oma(UFIC *c,int m,u32 a)
{
    WHAT_GROUP_UFICC *cc = (typeof(cc)) c->cc ;
    switch(m) {
    case UFIM_START:
 	log_printf(ls_what,"start count = %d\n",GDUCAST(a)->i) ;
	break ;
    case UFIM_FIRST:
	break ;
    case UFIM_NEXT: {
	WSC *wsc = cc->wsc ;
	json_t *e = (json_t *) c->next ;
	json_t *jid = json_object_get(e,"id") ;
	int id = json_integer_value(jid) ;
	log_printf(ls_what,"next (%d of %d) %d",c->iti,c->itn,id) ;
	hsr_printf(wsc->hsr,"<br>torrent %d",id) ;
	if (torrent_id_to_hash(id,0) == 0) {
	    log_printf_c(ls_what," adding\n") ;
	    cc->ufchild = dht_add_torrent_url_i(id) ;
	    cc->ufchild->parent = cc->ufparent ;
	    stimer_pause(c->st) ;
	    hsr_printf(wsc->hsr,"loading",id) ;
	    }
	else {
	    hsr_printf(wsc->hsr,"skipping",id) ;
	    log_printf_c(ls_what," skipping\n") ;
	    stimer_retrigger(c->st) ;
	    }
	break ;
	}
    case UFIM_LAST:
	break ;
    case UFIM_FINISH:
	iterator_close(c->it) ;
	json_iterator_destroy(c->it) ;
	hsr_close(cc->wsc->hsr) ;
	uf_destroy(cc->ufparent) ;
	log_printf(ls_what,"done\n") ;
	break ;
    }
    fflush(stdout) ;
    return(0) ;
    }

/* ================================================================ */
static u32 what_group_parent_uff(UF *uf,int m,u32 a)
{
    UFIC *c = (UFIC *) uf->d.v ;
    WHAT_GROUP_UFICC *cc ;
    switch(m) {
    case UFM_TYPENAME:
	return(mtputs((MT *) a,"WHAT_GROUP")) ;
    case UFM_DESTROY: {
	cc = (typeof(cc)) c->cc ;
	json_decref(cc->root) ;
	free(cc->wsc) ;
	free(cc) ;
	free(c) ;
	return 0 ;
	}
    case UFM_PRE_NOTIFY: {
	UFM_NOTIFY_ARG	*na = (UFM_NOTIFY_ARG *) a ;
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

extern void what_group_accept_response(UF *uf,RBP *r)
{
    HT		*ht = (typeof(ht)) uf->d.v ;
    json_t *ej = json_object_get(r->j,"status") ;
    json_t *rj = json_object_get(r->j,"response") ;
//    json_t *group = json_object_get(rj,"group") ;
    json_t *torrents = json_object_get(rj,"torrents") ;
    const char *s = json_string_value(ej) ;
//    json_dump_stdout(group) ;
//    json_dump_stdout(torrents) ;
//    json_dump_stdout(rj) ;
    (void) s ;
{
    WSC		*wsc = ((WSC *) ht->sc) ;
    WHAT_GROUP_UFICC *cc = tscalloc(1,WHAT_GROUP_UFICC) ;
    UFIC *c = tscalloc(1,UFIC) ;
    c->cc = cc ;
    cc->root = json_deep_copy(torrents) ;
    cc->ufparent = uf_create(what_group_parent_uff,c,0) ;
    cc->wsc = wsc ;
    json_array_iterator_open(c->it,cc->root,0) ;
    c->wait.before = 2000 ;
    c->wait.between = 5000 ;
    c->oma = what_group_ufic_oma ;
    ufi_start(c) ;
}
    return ;
}

static u32 what_group_uff(UF *uf,int m,u32 a)
{
    switch(m) {
    case UFM_TYPENAME:	return(mtputs((MT *) a,"what.group")) ;
    case UFM_HT_RESPONSE_JSON: {
	what_group_accept_response(uf,((RBP *) a)) ;
	break ;
	}
    default:
	break ;
	}
    return(wht_uff(uf,m,a)) ;
    }

extern void what_group(HSR *r)
{
    HT		*ht ;
    UF		*uf ;
    MT		mt[1] ;
    char	*group = hsr_arg_exists(r,"group") ;
    WSC		*wsc = tscalloc(1,WSC) ;
    if (!group) {
	errorshow("must supply a group id\n") ;
	return ;
	}
    MTALLOCA(mt,4096) ;
    mtputs(mt,"ajax.php?action=torrentgroup") ;
    mtprintf(mt,"&id=%s",group) ;
    ht = https_create("GET","what.cd",mt->s,0) ;
    ht->cookie = what_g.cookie ;
    wsc->hsr = r ;
    uf = uf_create(what_group_uff,ht,wsc) ;
    ufs_rc_add(uf) ;
    }


#ifdef __cplusplus /*Z*/
}
#endif
