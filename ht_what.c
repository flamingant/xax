#ifdef __cplusplus /*Z*/
extern "C" {
#endif

#include	<string.h>

#include	<sys/time.h>

/* ~# use collect ; collect::register_all('logsec','^LOGSEC (\w+)','$1') ; #~ */

/* ~# use decode ; #~ */

#include	"what.h"

#include	"common.h"

#include	".gen/ht_what.h"

LOGSEC ls_what[] = {"WHAT",1} ;

extern void json_dump_stdout(json_t *o)
{
    char *d = json_dumps(o,JSON_INDENT(2)) ;
    printf("%s\n",d) ;
    fflush(stdout) ;
    free(d) ;
}

/* ================================================================ */
/* ================================================================ */
WHAT_G what_g ;

extern u32 wht_uff(UF *uf,int m,u32 a)
{
    switch(m) {
    case UFM_HT_NOTREADY:
	return(!what_g.cookie) ;
    case UFM_HT_RESPONSE_BODY: {
	RBP	*p = (RBP *) a ;
	p->j = json_decode(p->mtu) ;
	uf_send(uf,UFM_HT_RESPONSE_JSON,(u32) p) ;
	json_decref(p->j) ;
	return 0 ;
	}
    default:
	break ;
	}
    return(ht_uff(uf,m,a)) ;
    }

/* ================================================================ */
static void what_header_response(UF *uf,HT *ht)
{
    MD		m[1] ;
    MD		mo[1] ;
    m->s = ht->response->s ;
    m->e = m->s + ht->rsp.obody_s ;
    if (header_find(m,mo,"Set-Cookie")) {
	char	*e = mo->s ;
	while (*e != ';') e++ ;
	what_g.cookie = memtocstring(mo->s,e-mo->s) ;
	}
    }
    
static u32 what_login_uff(UF *uf,int m,u32 a)
{
    HT		*ht = uf->d.ht ;
    switch(m) {
    case UFM_HT_NOTREADY:
	return(0) ;
    case UFM_TYPENAME:	return(mtputs((MT *) a,"what_login")) ;
    case UFM_HT_RESPONSE_HDR:
	what_header_response(uf,ht) ;
	break ;
    case UFM_HT_RESPONSE_JSON:
	break ;
    default:
	break ;
	}
    return(wht_uff(uf,m,a)) ;
    }

extern void what_login(HSR *r)
{
    HT		*ht ;
    UF		*uf ;
    ht = https_create("POST","what.cd","login.php","username=fishybishop&password=cm62lw54FB&login=Login") ;
    uf = uf_create(what_login_uff,ht,0) ;
    ufs_rc_add(uf) ;
    }

/* ================================================================ */
static u32 what_test_uff(UF *uf,int m,u32 a)
{
    switch(m) {
    case UFM_TYPENAME:	return(mtputs((MT *) a,"what_test")) ;
    case UFM_HT_RESPONSE_JSON: {
	json_t *jr = ((RBP *) a)->j ;
	int64 up,down ;
	double ratio ;
	if (!(jr = json_object_get(jr,"response"))) goto fail ;
	if (!(jr = json_object_get(jr,"userstats"))) goto fail ;
	up   = json_integer_value(json_object_get(jr,"uploaded")) ;
	down = json_integer_value(json_object_get(jr,"downloaded")) ;
	ratio = json_real_value(json_object_get(jr,"ratio")) ;
	printf("uploaded   = %-16llu\n",up) ;
	printf("downloaded = %-16llu\n",down) ;
	printf("up/down    = %-10.6f\n",((double) up) / down) ;
	printf("ratio      = %-10.6f\n",ratio) ;
fail:	
	break ;
	}
    default:
	break ;
	}
    return(wht_uff(uf,m,a)) ;
    }

extern void what_test(HSR *r)
{
    HT		*ht ;
    UF		*uf ;
    ht = https_create("GET","what.cd","ajax.php?action=index",0) ;
    ht->cookie = what_g.cookie ;
    uf = uf_create(what_test_uff,ht,0) ;
    ufs_rc_add(uf) ;
    }

/* ================================================================ */
/* ================================================================ */
/* ~~nametable("^extern void (what_(\\w+))\\(HSR",
   tag => "commands",
   transform => '"$2"',
   format => 'glt',
   )~~ */

extern void what_group(HSR *r) ;
extern void what_search(HSR *r) ;

/* ================================================================ */
extern void what_command(HSR *r,char *cmd)
{
    if (!cmd) return ;
    else if (!strcmp(cmd,"login")) what_login(r) ;
    else if (!strcmp(cmd,"test")) what_test(r) ;
    else if (!strcmp(cmd,"search")) what_search(r) ;
    else if (!strcmp(cmd,"group")) what_group(r) ;
    }
	
/* ================================================================ */
/* ================================================================ */
/* ~# use http ; #~ */

typedef struct {
    char	*url ;
    } C_WHAT ;

/* ~~hpf(help => "Invoke a what.cd command")~~ */
static u32 hpf_what(HSR *r,int m,u32 a)
{
    switch (m) {
    case HPM_CREATE:
	hsr_qlc_set(r,sizeof(C_WHAT)) ;
	break ;
    case HPM_INTERNAL:
	what_command(r,hsr_arg_exists(r,"cmd")) ;
	return 0 ;
    case HPM_CONTENT_SEND:
	return(0) ;
    case HPM_REQUEST_DONE:
	return(0) ;
    default:
	break ;
	}
    return(hpf_generic_ok(r,m,a)) ;
    }

#include	".gen/ht_what.c"

#ifdef __cplusplus /*Z*/
}
#endif
