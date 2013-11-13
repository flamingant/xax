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

#include	"common.h"

/* ================================================================ */
/* ================================================================ */
typedef struct {
    RCONS	*rc ;
    double	threshold ;
    char	*label ;
    } GLRTU ;

typedef struct {
    RCONS	*ru ;
    } GLRU ;

static void glr_tsg(HT *ht,TOD *tod)
{
    TSI		*tsi = tod->tsi_c ;
    GLRU	*sc = (GLRU *) ht->sc ;
    RCONS	*r ;

    for (r = sc->ru ; r ; r = r->cdr) {
	GLRTU	*tu = (GLRTU *) r->car ;
	if (tsi->ratio + .0005 >= tu->threshold) {
	    if (tu->label && strcmp(tu->label,tsi->label)) {
		tu->rc = rcons_cons(tod,tu->rc) ;
		printf("%s %9.6f %s ",tod->hash,tsi->ratio,tsi->label) ;
		printf("--> %s",tu->label) ;
		printf("\n") ;
	    }
	    break ;
	}
    }
}

extern void torrent_label(TOD *tod,char *label) ;

static void glr_tu_flush(HT *ht,GLRTU *tu)
{
    RCONS	*r ;
    printf("Label: %s\n",tu->label) ;
    for (r = tu->rc ; r ; r = r->cdr) {
	TOD	*tod = (TOD *) r->car ;
	printf("%s relabel -> %s\n",tod->hash,tu->label) ;
	torrent_label(tod,tu->label) ;
    }
    printf("\n") ;
    }

static void glr_tu_destroy(GLRTU *tu)
{
    RCONS	*r ;
    for (r = tu->rc ; r ; r = rcons_free(r)) {
    }
    }

static void glr_response_complete(UF *uf,HT *ht)
{
    GLRU	*sc = (GLRU *) ht->sc ;
    RCONS	*r ;
    for (r = sc->ru ; r ; r = r->cdr) {
	glr_tu_flush(ht,(GLRTU *) r->car) ;
	}
    }

static void glr_destroy(HT *ht)
{
    GLRU	*sc = (GLRU *) ht->sc ;
    RCONS	*r ;
    for (r = sc->ru ; r ; r = rcons_free(r)) {
	glr_tu_destroy((GLRTU *) r->car) ;
	}
    free(sc) ;
    }

static u32 get_label_ratios_uff(UF *uf,int m,u32 a)
{
    HT		*ht = uf->d.ht ;
    switch(m) {
    case UFM_TYPENAME:	return(mtputs((MT *) a,"STATUS_BY_LABEL")) ;
    case UFM_CREATE:
	ht->sc = (void *) a ;
	break ;
    case UFM_DESTROY:
	glr_destroy(ht) ;
	break ;
    case UFM_HT_RESPONSE_HDR:
	break ;
    case UFM_HT_RESPONSE_JSON:
	tlsi_accept_response(uf,ht,(RBP *) a) ;
	glr_response_complete(uf,ht) ;
	break ;
    case UFM_HT_TORRENT_STATUS_GET: 
	glr_tsg(ht,(TOD *) a) ;
	return(0) ;
    default:
	break ;
	}
    return(dht_uff(uf,m,a)) ;
    }

static void glrtu_add(GLRU *u,double threshold,char *label)
{
    GLRTU	*tu = tscalloc(1,GLRTU) ;
    tu->label = strdup(label) ;
    tu->threshold = threshold ;
    u->ru = rcons_cons(tu,u->ru) ;
    }

static void dht_get_label_ratios_glru(char *labels,GLRU	*u)
{
    MT		mt[1] ;
    UF		*uf ;
    char *t = "{\"method\":\"core.get_torrents_status\",\
\"params\":[\
{\"label\":[%s]},\
[\"label\",\"ratio\"]],\
\"id\":%%d\
}\
" ;
    MTALLOCA(mt,1024) ;
    mtprintf(mt,t,labels) ;
    uf = deluge_ht_create(get_label_ratios_uff,mt->s,(void *) u) ;
    ufs_rc_add(uf) ;
    }
    
static void dht_get_label_ratios(char *labels)
{
    GLRU	*u ;
    u = tscalloc(1,GLRU) ;

    glrtu_add(u,  1.0,"__plus") ;
    glrtu_add(u,  0.4,"__minus") ;
    glrtu_add(u,  0.0,"__oops") ;

    dht_get_label_ratios_glru(labels,u) ;
    }
    
extern void dht_relabel(char *label,char *style)
{
    MT		mt[1] ;
    MTALLOCA(mt,1024) ;
    qlistbuild(mt,1,&label) ;
    dht_get_label_ratios(mt->s) ;
    }

/* ================================================================ */
#include	"http.h"

/* ~# use http ; #~ */

static u32 hpf_relabel_content(HSR *r)
{
    char	*label ;
    char	*style ;
    if (!(label = hsr_arg_exists(r,"label")) || !*label)
	label = "mine" ;
    if (!(style = hsr_arg_exists(r,"style")) || !*style)
	style = "default" ;
    hsr_printf(r,"<h2>Relabel %s according to %s style</h2>",label,style) ;
    dht_relabel(label,style) ;
    return(0) ;
    }

/* ~~hpf()~~ */
static u32 hpf_torrent_relabel(HSR *r,int m,u32 a)
{
    switch (m) {
    case HPM_INTERNAL:
	return(hpf_relabel_content(r)) ;
    case HPM_CONTENT_SEND:
	hsr_printf(r,"Done") ;
	return(0) ;
    default:
	break ;
	}
    return(hpf_generic_ok(r,m,a)) ;
    }

/* ================================================================ */
static u32 torrent_label_per_ratio(void)
{
    MT		mt[1] ;
    MTALLOCA(mt,1024) ;
    GLRU	*u ;
    u = tscalloc(1,GLRU) ;
    vqlistbuild(mt,"-minus","1-plus","2-plus","3-plus",0) ;
    glrtu_add(u,0.0,"-minus") ;
    glrtu_add(u,1.0,"1-plus") ;
    glrtu_add(u,2.0,"2-plus") ;
    glrtu_add(u,3.0,"3-plus") ;
    glrtu_add(u,4.0,"4-plus") ;
    dht_get_label_ratios_glru(mt->s,u) ;
    return(0) ;
    }

/* ~~hpf()~~ */
static u32 hpf_torrent_label_per_ratio(HSR *r,int m,u32 a)
{
    switch (m) {
    case HPM_INTERNAL:
	return(torrent_label_per_ratio()) ;
    case HPM_CONTENT_SEND:
	hsr_printf(r,"Done") ;
	return(0) ;
    default:
	break ;
	}
    return(hpf_generic_ok(r,m,a)) ;
    }

/* ================================================================ */
#include	".gen/dht_label.c"

#ifdef __cplusplus /*Z*/
}
#endif
