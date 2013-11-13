#ifdef __cplusplus /*Z*/
extern "C" {
#endif

#include	<string.h>

#include	"pu/mt.h"

#include	<sys/time.h>

#include	"log.h"
/* ~# use collect ; collect::register_all('logsec','^LOGSEC (\w+)','$1') ; #~ */

#include	"ma.h"
#include	"dht.h"

#include	"uf.h"

#include	"tc_sql.h"

#include	"common.h"

/* ================================================================ */
/*
bool		has_incoming_connections
float 		upload_rate
float 		download_rate
float 		payload_upload_rate
float 		payload_download_rate
size_type 	total_download
size_type 	total_upload
size_type 	total_payload_download
size_type 	total_payload_upload
int 		num_peers
int 		dht_nodes
int 		dht_node_cache
int 		dht_torrents
*/
/* ================================================================ */
#include	"jsf.h"

typedef struct struct_SESTAT SESTAT ;

/*
~# use tstat ; #~

~~sopen("SESTAT")~~

~~tsf("total_payload_download",		"int64")~~
~~tsf("total_payload_upload",			"int64")~~
*/

#include	".gen/dht_session.h"

static struct {
    STIMER	*st ;
    int		running ;
    int		clock_stop ;
    SESTAT	stat[1] ;
    } g ;

/* ================================================================ */
extern void tstat_unpack(SF *f,void *p,json_t *value)
{
    for ( ; f->sft != SFT__END ; f++) {
	json_t *j = json_object_get(value, f->name) ;
	if (!j) continue ;
	jsf_read(f,p,j) ;
	}
    }

extern void sestat_unpack(SESTAT *stat,json_t *value)
{
    tstat_unpack(SESTAT_sf,stat,value) ;
    }

LOGSEC ls_session[] = {"SESSION",1} ;

static void json_accept(UF *uf,HT *ht,RBP *r)
{
    json_t *ej = json_object_get(r->j, "error") ;
    json_t *rj = json_object_get(r->j, "result") ;
    const char *es = json_string_value(ej) ;
    if (es) {
	errorshow((char *) es) ;
	return ;
	}
{
    SESTAT	stat[1] ;
    sestat_unpack(stat,rj) ;
    if (stat->total_payload_download != g.stat->total_payload_download ||
	stat->total_payload_upload != g.stat->total_payload_upload) {
	log_printf(ls_session,"down: %-12lld +%-6lld up: %-12lld +%-6lld\n",
		   stat->total_payload_download,
		   stat->total_payload_download - g.stat->total_payload_download,
		   stat->total_payload_upload,
		   stat->total_payload_upload - g.stat->total_payload_upload
		   ) ;
	g.stat[0] = stat[0] ;
	}
    }
}

/* ~# use collect ; collect::register('uff','dht_get_session_status_uff') ; #~ */

UFSS dht_get_session_status_ufss[] = {"session"} ;

extern u32 dht_get_session_status_uff(UF *uf,int m,u32 a)
{
    HT		*ht = uf->d.ht ;
    switch(m) {
    case UFM_GET_STATIC:
	return (u32) dht_get_session_status_ufss ;
    case UFM_TYPENAME:	return(mtputs((MT *) a,"SESSION")) ;
    case UFM_HT_RESPONSE_HDR:
	break ;
    case UFM_HT_RESPONSE_JSON:
	json_accept(uf,ht,(RBP *) a) ;
	break ;
    default:
	break ;
	}
    return(dht_uff(uf,m,a)) ;
    }

extern void dhtm_put_method(MT *mt,char *method)
{
    mtprintf(mt,"{\"method\":\"%s\"",method) ;
    }

extern void param_element_add(MT *mt,char *element,int comma)
{
    mtprintf(mt,"\"%s\"%s",element,comma ? "," : "") ;
    }

static void dht_get_session_status(void)
{
    MT		mt[1] ;
    UF		*uf ;
    MTALLOCA(mt,DHT_MAX) ;
    dhtm_put_method(mt,"core.get_session_status") ;
    mtputs(mt,",\"params\":[[") ;
    param_element_add(mt,"total_payload_download",1) ;
    param_element_add(mt,"total_payload_upload",0) ;
    mtputs(mt,"]],\"id\":%d}") ;
    uf = deluge_ht_create(dht_get_session_status_uff,mt->s,0) ;
    ufs_rc_add(uf) ;
    }
    
static u32 wakeup_st(STIMER *st,int m,u32 a)
{
    switch(m) {
    case STM_EXPIRE:
	if (g.clock_stop) stimer_clock_stop() ;
	dht_get_session_status() ;
	return(1) ;
	}
   return(0) ;
}

/* ================================================================ */
static void dht_session_run_start(void)
{
    if (g.running) return ;
    g.st = stimer_add(wakeup_st,0,1000) ;
    g.st->state |= STS_REARM ;
    g.running = 1 ;
    g.clock_stop = 0 ;
}

static void dht_session_run_stop(void)
{
    if (g.st) {
	stimer_kill(g.st) ;
	g.st = 0 ;
	}
    g.running = 0 ;
    }

static void dht_session_run_toggle(void)
{
    if (!g.running)
	 dht_session_run_start() ;
    else dht_session_run_stop() ;
    }

/* ================================================================ */
extern u32 dht_session_rsf(int m,u32 a)
{
    u32		r ;
    switch(m) {
    case RSM_START:
	dht_session_run_start() ;
	break ;
    case RSM_STOP:
	dht_session_run_stop() ;
	break ;
    case RSM_TOGGLE:
	dht_session_run_toggle() ;
	break ;
    case RSM_GETSTATE:
	return(g.running) ;
    case RSM_SETSTATE:
	r = g.running ;
	if (a)
	     dht_session_run_start() ;
	else dht_session_run_stop() ;
	return(r) ;
	}
    return(0) ;
    }

/* ================================================================ */
#include	<pu/exithook.h>

static void dht_session_unplug_exithook(void)
{
    dht_session_run_stop() ;
    }

extern void dht_session_unplug(void)
{
    exithook_run_now((EXITHOOK) dht_session_unplug_exithook,0) ;
    }

extern void dht_session_plugin(void)
{
    exithook_install((EXITHOOK) dht_session_unplug_exithook,0) ;
    }

/* ================================================================ */
/* ================================================================ */
#include	"ht_tc.h"
#include	"http.h"

/* ~# use http ; #~ */

static u32 dht_session_state_vcf(void *c,int m,u32 a)
{
    VCFU *u = (VCFU *) &a ;
    switch(m) {
    case VCF_SETCHAR:
	return(dht_session_state_vcf(c,switch_state_read(u->s),0)) ;
    case VCF_GETCHAR:
	switch_state_write(u->mt,dht_session_rsf(RSM_GETSTATE,0)) ;
	return 0 ;
    case VCF_SWITCH_OFF:
    case VCF_SWITCH_ON:
    case VCF_SWITCH_TOGGLE:
	dht_session_rsf(m,a) ;
	return(0) ;
    case VCF_GET_NAME:
	return(u32) "session" ;
    case VCF_GET_STATE:
	return (u32) (g.running ? "on" : "off") ;
	}
    return(0) ;
    }

/* ~~hpf(type => "state",help => "Set session stats monitor process state")~~ */
static u32 hpf_session(HSR *r,int m,u32 a)
{
    switch (m) {
    case HPM_INTERNAL:
{
    char	*state_cmd ;
    if (!(state_cmd = hsr_arg_exists(r,"state"))) {
	state_cmd = "toggle" ;
	}
    vcf_apply(dht_session_state_vcf,DUMMY_VCC,VCF_SETCHAR,(u32) state_cmd) ;
    return(0) ;
    }
    case HPM_CONTENT_SEND:
	hsr_vcf_pm(r,"%s",dht_session_state_vcf,DUMMY_VCC,VCF_GETCHAR) ;
	return(0) ;
    default:
	break ;
	}
    return(hpf_generic_ok(r,m,a)) ;
    }

#include	".gen/dht_session.c"

#ifdef __cplusplus /*Z*/
}
#endif
