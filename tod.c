#ifdef __cplusplus /*Z*/
extern "C" {
#endif

#include	"talloc.h"
#include	<string.h>
#include	<stdarg.h>
#include	<stdlib.h>
#include	<pu/mt.h>

#include	"log.h"
#include	"tod.h"

#define HTD_TYPE	TOD

#include	"hashtab.h"

static HASHTAB tod_tab[1] ;

static TOD *tod_alloc(void)
{
    return((TOD *) calloc(1,sizeof(TOD))) ;
    }

extern TOD *tod_intern(char *hash)
{
    HTC **rslot ;
    HTC *c = htc_find(tod_tab,hash,40,&rslot) ;
    TOD *d ;
    if (c) {
	return(c->d) ;
    }
    else {
	c = htc_alloc() ;
	c->next = *rslot ;
	*rslot = c ;
	c->d = d = tod_alloc() ;
	d->tid = -1 ;
	d->tsi_c = tscalloc(1,TSI) ;
	d->hash = strdup(hash) ;
	return(d) ;
	}
    }

static void tod_destroy(TOD *tod)
{
    free(tod->hash) ;
    tsi_destroy(tod->tsi_c) ;
    free(tod->tsi_c) ;
    free(tod) ;
    }

static int tod_keycmp(HTD *d,char *pb,int cb)
{
    return(memcmp(d->hash,pb,cb)) ;
    }

extern void tod_tab_init(void)
{
    tab_slot_inits(tod_tab,63) ;
    tod_tab->keycmp = tod_keycmp ;
    }

extern void tod_tab_map(void (*fun)(TOD *,u32),u32 a)
{
    hashtab_map(tod_tab,fun,a) ;
}

/* ================================================================ */
static void tod_tab_destroy_ttmf(TOD *t,u32 a)
{
    tod_destroy(t) ;
    }

extern void tod_tab_destroy(void)
{
    tod_tab_map(tod_tab_destroy_ttmf,0) ;
    hashtab_destroy(tod_tab) ;
}

/* ================================================================ */
#define TIPREFIX	"::name::"
#define THASHLEN	40
	
extern int tod_identify(MT *mt,TOD *t)
{
    if (!mt->s) return(sizeof(TIPREFIX)+THASHLEN) ;
{
    char	*s = mt->c ;
    if (t->tsi_c && t->tsi_c->name) {
	char	*c ;
	mtputs(mt,"::name::") ;
	c = mt->c ;		/* this must go after the first mtputs */
	mtprintf(mt,"%-40s",t->tsi_c->name) ;
	if (strlen(c) > THASHLEN) {
	     mt->c = c + THASHLEN ;
	     }
	mtputs(mt,":: ") ;
	}
    else {
	mtprintf(mt,"::hash::%s:: ",t->hash) ;
	}
    return(mt->c-s) ;
    }
    }

extern void tod_log_printf_va(LOGSEC *ls,TOD *t,char *fmt,va_list va)
{
    MT		mt[1] ;
    MTALLOCA(mt,8192) ;
    tod_identify(mt,t) ;
    mtvprintf(mt,fmt,va) ;
    log_puts(ls,mt->s) ;
    }

extern void tod_log_printf(LOGSEC *ls,TOD *t,char *fmt,...)
{
    va_list	va ;
    va_start(va,fmt) ;
    tod_log_printf_va(ls,t,fmt,va) ;
    }

/* ================================================================ */
#include	"jsf.h"

/*
~# use tstat ; #~
~~sopen("TSI")~~
~~tsf("queue",			"int")~~
~~tsf("name",			"char *")~~
~~tsf("total_size",		"int")~~
~~tsf("state",			"ENUM_TORRENTSTATE")~~
~~tsf("progress",		"double")~~
~~tsf("num_seeds",		"int")~~
~~tsf("total_seeds",		"int")~~
~~tsf("num_peers",		"int")~~
~~tsf("total_peers",		"int")~~
~~tsf("download_payload_rate",	"int")~~
~~tsf("upload_payload_rate",	"int")~~
~~tsf("eta",			"int")~~
~~tsf("ratio",			"double")~~
~~tsf("distributed_copies",	"double")~~
~~tsf("is_auto_managed",	"bool")~~
~~tsf("time_added",		"double")~~
~~tsf("tracker_host",		"char *")~~
~~tsf("save_path",		"char *")~~
~~tsf("total_done",		"int64")~~
~~tsf("total_uploaded",		"int64")~~
~~tsf("max_download_speed",	"int")~~
~~tsf("max_upload_speed",	"int")~~
~~tsf("seeds_peers_ratio",	"double")~~
~~tsf("label",			"char *")~~
*/

#include	".gen/tod.c"
/* ================================================================ */
extern void tsi_unpack(TSI *tsi,json_t *value)
{
    SF	*f ;
    for (f = TSI_sf ; f->sft != SFT__END ; f++) {
	json_t *j = json_object_get(value, f->name) ;
	if (!j) continue ;
	jsf_read(f,tsi,j) ;
	}
    }

extern void tsi_destroy(TSI *tsi)
{
    SF	*f ;
    for (f = TSI_sf ; f->sft != SFT__END ; f++) {
	jsf_free(f,tsi) ;
	}
    }


#ifdef __cplusplus /*Z*/
}
#endif
