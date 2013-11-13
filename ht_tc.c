#ifdef __cplusplus /*Z*/
extern "C" {
#endif

#include	"errors.h"

#include	<string.h>

#include	<pu/mt.h>

#include	<pu/rcons.h>
#include	<pu/regex.h>

#include	"uf.h"
#include	"ma.h"

#include	"dht.h"

#include	"sql.h"
#include	"tc_sql.h"

#include	"common.h"

#include	"ht_tc.h"

/* ================================================================ */
#include	"http.h"

/* ~# use http ; #~ */

/* ================================================================ */
#include	"vcf.h"
#include	"jsf.h"

extern void hsr_vcf_pm(HSR *r,char *fmt,VCFFUN fun,void *c,int m)
{
    MT	mt[1] ;
    MTALLOCA(mt,1024) ;
    fun(c,m,(u32) mt) ;
    hsr_printf(r,fmt,mt->s) ;
    }

extern void hsr_vcf_element_state(HSR *r,VCFFUN fun,void *c,char *element)
{
    MT	mt[1] ;
    MTALLOCA(mt,1024) ;
    fun(c,VCF_GETCHAR,(u32) mt) ;		/* send the VCF_GETCHAR */
{
    char	*js ;
    json_t *e ;
    e = json_object() ;
    json_object_set_new(e,"element",	json_string(element)) ;
    json_object_set_new(e,"state",		json_string(mt->s)) ;
    js = json_dumps(e,0) ;
    hsr_printf(r,"%s",js) ;
    free(js) ;
    json_decref(e) ;
    }
    }

/* ================================================================ */
#define TIMER_VCC	DUMMY_VCC

static u32 timer_state_vcf(void *c,int m,u32 a)
{
    VCFU *u = (VCFU *) &a ;
    switch(m) {
    case VCF_SETCHAR:
	return(timer_state_vcf(c,switch_state_read(u->s),0)) ;
    case VCF_GETCHAR:
	switch_state_write(u->mt,stimer_clock_started()) ;
	return 0 ;
    case VCF_SWITCH_ON:
	stimer_clock_start() ;
	return(0) ;
    case VCF_SWITCH_OFF:
	stimer_clock_stop() ;
	return(0) ;
    case VCF_SWITCH_TOGGLE:
	if (stimer_clock_started())
	     stimer_clock_stop() ;
	else stimer_clock_start() ;
	return(0) ;
    case VCF_GET_NAME:
	return(u32) "timer" ;
    case VCF_GET_STATE:
	return (u32) (stimer_clock_started() ? "on" : "off") ;
	}
    return(0) ;
    }

/* ~~hpf(type => "state",help => "Set timer run state")~~ */
static u32 hpf_timer(HSR *r,int m,u32 a)
{
    switch (m) {
    case HPM_INTERNAL:
{
    char	*state_cmd ;
    state_cmd = hsr_arg_or_toggle(r,"state") ;
    vcf_apply(timer_state_vcf,TIMER_VCC,VCF_SETCHAR,(u32) state_cmd) ;
    return(0) ;
    }
    case HPM_CONTENT_SEND:
	hsr_vcf_element_state(r,timer_state_vcf,TIMER_VCC,"Timer") ;
	return(0) ;
    default:
	break ;
	}
    return(hpf_generic_ok(r,m,a)) ;
    }

/* ================================================================ */
typedef struct {
    char	*torrent_id ;
   } TORRENT_ADD_ARGS ;

/* ~~hpf(help => "Add torrent by id")~~ */
static u32 hpf_torrent_start(HSR *r,int m,u32 a)
{
    switch (m) {
    case HPM_CONTENT_SEND:
{
    TORRENT_ADD_ARGS args ;
    args.torrent_id = hsr_arg_exists(r,"id") ;
    dht_add_torrent_url(args.torrent_id) ;
    hsr_printf(r,"<h2>%s wants to start %s</h2>",r->url->s,args.torrent_id) ;
    return(0) ;
    }
    default:
	break ;
	}
    return(hpf_generic_ok(r,m,a)) ;
    }

/* ================================================================ */
extern u32 thcp_state_vcf(void *c,int m,u32 a) ;

/* ~~hpf(type => "state",help => "Set torrent health check process state")~~ */
static u32 hpf_check(HSR *r,int m,u32 a)
{
    switch (m) {
    case HPM_INTERNAL: {
	char	*state_cmd ;
	state_cmd = hsr_arg_or_toggle(r,"state") ;
	thcp_state_vcf(DUMMY_VCC,VCF_SETCHAR,(u32) state_cmd) ;
	return 0 ;
	}
    case HPM_CONTENT_SEND:
	hsr_vcf_element_state(r,thcp_state_vcf,DUMMY_VCC,"Check") ;
	return(0) ;
    default:
	break ;
	}
    return(hpf_generic_ok(r,m,a)) ;
    }

/* ================================================================ */
typedef struct {
    MT		filter[1] ;
    u32		mask ;
    } GAS_C ;

static void get_all_status_internal(HSR *r,GAS_C *c)
{
    dht_get_status_filter_mask(0,c->filter->s,c->mask) ;
    }

static void filter_try(HSR *r,char *name)
{
    GAS_C	*c = (GAS_C *) r->lc ;
    char	*value ;
    if (value = hsr_arg_exists(r,name)) {
	param_add(c->filter,name,value) ;
	}
    }

/* ================================================================ */
/* ~~hpf(help => "Get state of all torrents")~~ */
static u32 hpf_get_all_status(HSR *r,int m,u32 a)
{
    GAS_C	*c = (GAS_C *) r->lc ;
    switch (m) {
    case HPM_CREATE:
	r->lc = c = tscalloc(1,GAS_C) ;
	mtmalloc(c->filter,1024) ;
	filter_try(r,"label") ;
	filter_try(r,"state") ;
	c->mask = tsi_fname_list_to_mask(hsr_arg_exists(r,"fields")) ;
	*c->filter->c = 0 ;
	break ;
    case HPM_DESTROY:
	mtfree(c->filter) ;
	free(c) ;
	r->lc = 0 ;
	break ;
    case HPM_INTERNAL:
	get_all_status_internal(r,c) ;
	break ;
    case HPM_CONTENT_SEND:
	hsr_printf(r,"<h2>Done (%s)</h2>",r->url->s) ;
	return(0) ;
	}
    return(hpf_generic_ok(r,m,a)) ;
    }

/* ================================================================ */
/* the apply feature is not done like a programming language
   especially not lisp. It is done by simulated visits to the
   page (command) with a torrent hash equal to each of the hashes
   returned from the query.
   Having a torrent object inside a lisp system is feasable.
   In this case the determination of whether the function can
   be applied to torrent objects is handled by the lisp function.
   In that case it is still a complication that the result is
   returned asynchronously, and we would still need a way of
   catching and interpreting the result. The existing lisp
   system would find that hard unless we have a robust way of
   saving the state from request to response without relying on
   any global lisp symbol values or other shared objects
   */

typedef struct {
    HSR		ri[1] ;		/* inner HSR */
    MT		mt[1] ;
    char	*query ;
    char	*extra ;
    char	*fname ;
    char	*error ;
    sqlite3_stmt *stmt ;
    } APPLY_C ;

static void hpf_apply_prepare(HSR *r,APPLY_C *c)
{
    char	*value ;
    c->query = c->mt->c ;
    mtputs(c->mt,"select hash from tstatus") ;
    if (value = hsr_arg_exists(r,"where")) {
	mtprintf(c->mt," where %s",value) ;
	}
    if (c->fname = hsr_arg_exists(r,"action")) {
	hsr_bind_url(c->ri,c->fname) ;
	if (!c->ri->hpo || c->ri->hpo == hpo_not_found) {
	    c->error = "Unknown function" ;
	    return ;
	    }
	if (!hsr_osend(c->ri,HPM_TORRENT_APPLY_OK,0)) {	
	    c->error = "Function cannot be applied to a torrent" ;
	    return ;
	}
    }
    if (value = hsr_arg_exists(r,"extra")) {
	c->extra = value ;
	}
    else c->extra = "" ;
    }

static void hpf_apply_internal(HSR *r,APPLY_C *c)
{
    hpf_apply_prepare(r,c) ;
{
    MT mtx[1] ;
    int sr ;
    MTALLOCA(mtx,1<<16) ;
    sr = sqlite3_prepare_v2(sqlgp()->dbh,c->query,-1,&c->stmt,0) ;
    if (sr != SQLITE_OK) {
	c->error = "sql fail" ;
	return ;
	}
    while (1) {
	sr = sqlite3_step_or(OR_WARN,sqlgp()->dbh,c->stmt) ;
	if (sr == SQLITE_ROW) {
	    const uchar *hash = sqlite3_column_text(c->stmt,0) ;
	    printf("apply %s to %s\n",c->fname,hash) ;
	    mtprintf(mtx,"%s?hash=%s%s",c->fname,hash,c->extra) ;
	    /* not using internal HSR here ?? */
	    hsr_execute(mtx->s) ;
	    MTREWIND(mtx) ;
	    }
	else break ;
	}
    sqlite3_finalize(c->stmt) ;
    }
}

/* ~~hpf(help => "Apply a torrent control function to a queried set of torrents")~~ */
static u32 hpf_apply(HSR *r,int m,u32 a)
{
    APPLY_C	*c = (APPLY_C *) r->lc ;
    switch (m) {
    case HPM_CREATE:
	r->lc = c = tscalloc(1,APPLY_C) ;
	mtmalloc(c->mt,1<<16) ;
	break ;
    case HPM_DESTROY:
	mtfree(c->mt) ;
	free(c) ;
	r->lc = 0 ;
	break ;
    case HPM_INTERNAL:
	hpf_apply_internal(r,c) ;
	break ;
    case HPM_CONTENT_SEND:
	hsr_printf(r,"<h2>Apply Complete (%s) (%s)</h2>",r->url->s,c->error ? c->error : "OK") ;
	return(0) ;
	}
    return(hpf_generic_ok(r,m,a)) ;
    }

/* ================================================================ */
typedef struct {
    char	*error ;
    char	*hash ;
    TOD		*tod ;
   } TA_KILL_C ;

static void thmi_remove_db_aware(char *hash,char *params)
{
    thmi_remove(hash,params) ;
    }

/* ~~hpf(help => "Kill a torrent by hash")~~ */
static u32 hpf_torrent_kill(HSR *r,int m,u32 a)
{
    TA_KILL_C	*c = (TA_KILL_C *) r->lc ;
    switch (m) {
    case HPM_TORRENT_APPLY_OK:
	return(1) ;
    case HPM_DESTROY:
	free(c) ;
	break ;
    case HPM_CREATE:
	r->lc = c = tscalloc(1,TA_KILL_C) ;
	if (!(c->hash = hsr_arg_exists(r,"hash"))) {
	    c->error = "No torrent hash supplied" ;
	    return(0) ;
	    }
	break ;
    case HPM_INTERNAL:
//	thmi_remove_db_aware(c->hash,"true") ;
	thmi_pause(c->hash) ;
	break ;
    case HPM_CONTENT_SEND:
{
    hsr_printf(r,"<h2>Kill torrent (%s)</h2>",c->hash) ;
    return(0) ;
    }
    default:
	break ;
	}
    return(hpf_generic_ok(r,m,a)) ;
    }

/* ================================================================ */
typedef struct {
    char	*error ;
    char	*hash ;
    char	*label ;
   } TA_LABEL_C ;

extern void torrent_label(TOD *tod,char *label) ;
extern TOD *tod_intern(char *hash) ;

/* ~~hpf(help => "Apply label to torrent")~~ */
static u32 hpf_torrent_label(HSR *r,int m,u32 a)
{
    TA_LABEL_C	*c = (TA_LABEL_C *) r->lc ;
    switch (m) {
    case HPM_TORRENT_APPLY_OK:
	return(1) ;
    case HPM_DESTROY:
	free(c) ;
	break ;
    case HPM_CREATE:
	r->lc = c = tscalloc(1,TA_LABEL_C) ;
	if (!(c->hash = hsr_arg_exists(r,"hash"))) {
	    c->error = "No torrent hash supplied" ;
	    return(0) ;
	    }
	if (!(c->label = hsr_arg_exists(r,"label"))) {
	    c->error = "No label supplied" ;
	    return(0) ;
	    }
	break ;
    case HPM_INTERNAL: {
	TOD *tod ;
	tod = tod_intern(c->hash) ;
	torrent_label(tod,c->label) ;
	return(0) ;
	}
    case HPM_CONTENT_SEND:
{
    hsr_printf(r,"<h2>Label (%s) torrent (%s)</h2>",c->label,c->hash) ;
    return(0) ;
    }
    default:
	break ;
	}
    return(hpf_generic_ok(r,m,a)) ;
    }

/* ================================================================ */
static u32 hpf_stub(HSR *r,int m,u32 a)
{
    typedef struct {
	char	*error ;
    } HPF_STUB_C ;

    HPF_STUB_C	*c = (HPF_STUB_C *) r->lc ;
    switch (m) {
    case HPM_CREATE:
	r->lc = c = tscalloc(1,HPF_STUB_C) ;
	break ;
    case HPM_DESTROY:
	free(c) ;
	break ;
    case HPM_INTERNAL: {
	return(0) ;
	}
    case HPM_CONTENT_SEND: {
	return(0) ;
    }
    default:
	break ;
	}
    return(hpf_generic_ok(r,m,a)) ;
    }

/* ================================================================ */
#include	".gen/ht_tc.c"


#ifdef __cplusplus /*Z*/
}
#endif
