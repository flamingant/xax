#ifdef __cplusplus /*Z*/
extern "C" {
#endif

#include	"arg.h"
#include	"dp.h"

#include	<unistd.h>
#include	<stdio.h>
#include	<sys/socket.h>
#include	<arpa/inet.h>
#include	<stdlib.h>
#include	<netdb.h>
#include	<string.h>
#include	<sys/stat.h>

#include	<fcntl.h>
#include	<errno.h>

#include	<sys/select.h>
#include	<sys/param.h>

#include	<stdarg.h>
#include	"pu/mt.h"

#include	"pu/rcons.h"
#include	"pu/regex.h"

#include	"common.h"

#include	".gen/http.h"

/* ================================================================ */
#include	"uf.h"
#include	"http.h"

typedef struct {
    char	*document_root ;
    int		runstate ;
    int		nostart ;
    REGEX	geturl[1] ;
    REGEX	posturl[1] ;
    } HSG ;

static HSG g_hsg = {
    "../htdocs",
    } ;

typedef struct {
    int		port ;
    } HSS_CREATE ;

HSS_CREATE hssc_default[1] = {
    5007,
    } ;

/* ================================================================ */
#define HSR_FSEND(r,m,a)	r->_.hpf(r,m,a)

extern u32 hsr_fsend(HSR *r,int m,u32 a)
{
    return(HSR_FSEND(r,m,a)) ;
    }

extern u32 hsr_osend(HSR *r,int m,u32 a)
{
    return(r->hpo->f(r,m,a)) ;
    }

extern void hsr_obind(HSR *r,HPO *o)
{
    if (!o) {
	/* this should be caught before this */
	o = hpo_not_found ;
	}
    r->hpo = o ;
    r->_.hpf = o->f ;
    }

/* ================================================================ */
extern void *hsr_qlc(HSR *r,int size)
{
    if (size > MAXQLC)
	errorfatal("trying to allocate %d bytes in quick local context (max %d)",size,MAXQLC) ;
    return(r ? r->qlc : 0) ;
    }

extern void hsr_qlc_set(HSR *r,int size)
{
    if (size > MAXQLC)
	errorfatal("trying to allocate %d bytes in quick local context (max %d)",size,MAXQLC) ;
    r->lc = r->qlc ;
    }

/* ~# use http ; #~ */

static void hss_state_clear(HSS *hss,int state)
{
    hss->state &= ~state ;
    }

static void hss_state_set(HSS *hss,int state)
{
    hss->state |= state ;
    }

static void hsr_state_clear(HSR *r,int state)
{
    r->state &= ~state ;
    }

static void hsr_state_set(HSR *r,int state)
{
    r->state |= state ;
    }

#include	"log.h"

/* ~# use collect ; collect::register_all('logsec','^LOGSEC (\w+)','$1') ; #~ */

LOGSEC lc_hsr_put[] = {"HSR_PUT",0} ;

extern void hsr_put(HSR *r,char *pb,int cb)
{
    int		n ;
    if ((n = write(r->csock,pb,cb)) < 0) {
	hsr_state_set(r,HSR_WRITE_FAIL) ;
	}
    log_write(lc_hsr_put,pb,cb) ;
    }

extern void hsr_puts(HSR *r,char *s)
{
    return(hsr_put(r,s,strlen(s))) ;
    }

extern void hsr_printf(HSR *r,char *fmt,...)
{
    MT		mt[1] ;
    va_list	va ;
    va_start(va,fmt) ;
    MTALLOCA(mt,65536) ;
    mtvprintf(mt,fmt,va) ;
    hsr_put(r,mt->s,MTFillSize(mt)) ;
    }

static void hsr_request_read_slice(HSR *r)
{
    int		csock = r->csock ;
    MT		*mt = r->request ;
    int		n ;
    int		slice = 65536 ;

    if (mt->c + slice > mt->e) {
	int size = mt->s ? MTAllSize(mt) * 4 : slice * 4 ;
	mtrealloc(mt,size) ;
    }
    n = recv(csock, mt->c, slice, 0) ;
    if (n < 0) {
	hsr_state_set(r,HSR_REQUEST_FAIL) ;
	return ;
    }
    if (n == 0) {
	hsr_state_set(r,HSR_REQUEST_COMPLETE) ;
	return ;
    }
    *(mt->c += n) = 0 ;
    }

static void hsr_request_read_loop(HSR *r)
{
    hsr_request_read_slice(r) ;
    }

static int hsr_accept(HSS *s,HSR *r)
{
    int clilen ;
    struct sockaddr_in  cli_addr;

    clilen = sizeof(cli_addr);

    r->csock = accept(s->sockfd,(struct sockaddr *)&cli_addr,&clilen);
    if (r->csock < 0) {
        hsr_state_set(r,HSR_ACCEPT_FAIL) ;
    }
    return(r->csock) ;
    }

extern char *http_status_name(int status)
{
    switch(status) {
    case 200:	return("OK") ;
    case 301:   return("Moved Permanently") ;
    case 302:   return("Found") ;
    case 404:   return("Not Found") ;
    default:	return("??") ;
    }
    }

static void hsr_send_status(HSR *r)
{
    int status = r->http.status ;
    hsr_printf(r,"HTTP/1.1 %d %s\r\n",status,http_status_name(status)) ;
    }

extern void hsr_response_hdr(HSR *r)
{
    hsr_send_status(r) ;
    hsr_puts(r,"Host: boo\r\n");
    hsr_puts(r,"Server: foohttpd/0.1.0\r\n");
    hsr_printf(r,"Content-type: %s\r\n",r->content_type);
    hsr_puts(r,"\r\n") ;
    }

/* ================================================================ */
#include	<pu/filename.h>

extern char *content_type_guess(char *name,char *data,int size)
{
    char *suffix = filename_suffix(name) ;
    if (!stricmp(suffix,".js")) return "application/javascript" ;
    if (!stricmp(suffix,".css")) return "text/css" ;
    if (!stricmp(suffix,".ico")) return "image/x-icon" ;
    return("text/html") ;
    }

/* ================================================================ */
extern int url_match(HSR *r,char *s)
{
    if (!r->url->s) return 0 ;
    return(!strcmp(r->url->s,s)) ;
    }
    
extern int url_match_root(HSR *r,char *s)
{
    if (!r->url->s) return 0 ;
    if (r->url->s[0] != '/') return 0 ;
    return(!strcmp(r->url->s+1,s)) ;
    }
    
/* ================================================================ */
/* ~~hpf(help => "Quit the application")~~ */
static u32 hpf_quit(HSR *r,int m,u32 a)
{
    switch (m) {
    case HPM_CONTENT_SEND:
	hsr_printf(r,"<h2>Bye!</h2>") ;
	ufs_quit() ;
	break ;
    default:
	break ;
	}
    return(hpf_generic_ok(r,m,a)) ;
    }

/* ================================================================ */
/* ~~hpf()~~ */
static u32 hpf_favicon(HSR *r,int m,u32 a)
{
    switch (m) {
    case HPM_GET_NAME:
	return((u32) "favicon.ico") ;
    case HPM_HEADER_SEND:
	r->http.status = 302 ; 
	hsr_send_status(r) ;
	hsr_printf(r,"Location: http://boo%s\r\n\r\n",r->url->s) ;
	return(0) ;
    case HPM_CAN_VISIT:
	return(0) ;
    default:
	break ;
	}
    return(hpf_null(r,m,a)) ;
    }

/* ================================================================ */
static void htdoc_filename_build(MT *mt,HSR *r)
{
    mtprintf(mt,"%s%s",g_hsg.document_root,r->url->s) ;
    if (mt->c[-1] == '/')
	mtprintf(mt,"index.html") ;
    }

static int htdoc_url_match(HSR *r)
{
    struct stat st[1] ;
    MT		mt[1] ;
    MTALLOCA(mt,strlen(r->url->s)+64) ;
    htdoc_filename_build(mt,r) ;
    if (stat(mt->s, st) == -1)
	 return(0) ;
    else return(1) ;
    }
    
static void htdoc_content_send(HSR *r)
{
    struct stat st[1];
    MT		mt[1] ;
    MT		mtd[1] ;
    FILE	*f ;
    int		n ;
    MTALLOCA(mt,strlen(r->url->s)+64) ;
    htdoc_filename_build(mt,r) ;

    /* we already 'know' the file exists */

    stat(mt->s,st) ;
    f = fopen(mt->s,"r") ;
    mtmalloc(mtd,st->st_size) ;
    n = read(fileno(f),mtd->s,st->st_size) ;

    r->content_type = content_type_guess(mt->s,mtd->s,st->st_size) ;
    r->http.status = 200 ;
    hsr_response_hdr(r) ;

    hsr_put(r,mtd->s,n) ;
    mtfree(mtd) ;
    fclose(f) ;
    }
    
/* ~~hpf()~~ */
static u32 hpf_htdoc(HSR *r,int m,u32 a)
{
    switch (m) {
    case HPM_URL_MATCH:
	if (htdoc_url_match(r))
	    return(1) ;
	else return(0) ;	/* page cannot be matched by its own name */
    case HPM_HEADER_SEND:
	/* wait until we look at the content */
	return(0) ;
    case HPM_CONTENT_SEND:
	htdoc_content_send(r) ;
	break ;
    case HPM_CAN_VISIT:
	return(0) ;
    default:
	break ;
	}
    return(hpf_generic_ok(r,m,a)) ;
    }

extern HPO hpo_htdoc[] ;
/* ================================================================ */
/* ~~hpf()~~ */
extern u32 hpf_not_found(HSR *r,int m,u32 a)
{
    switch (m) {
    case HPM_GET_NAME:
	return(0) ;
    case HPM_URL_MATCH:
	return(0) ;		/* page cannot be matched by its own name */
    case HPM_HEADER_SEND:
	r->http.status = 404 ;
	hsr_response_hdr(r) ;
	return(0) ;
    case HPM_CONTENT_SEND:
	hsr_printf(r,"<h2>That page '%s' does not exist. Sorry</h2>",r->url->s) ;
	return(0) ;
    default:
	return(hpf_null(r,m,a)) ;
	}
}

extern HPO hpo_not_found[] ;
/* ================================================================ */
extern u32 hpf_generic_ok(HSR *r,int m,u32 a)
{
    switch (m) {
    case HPM_URL_MATCH:
    case HPM_CONTENT_SEND:
	/* these should usually be processed by the subclass */
	break ;
    case HPM_HEADER_SEND:
	r->http.status = 200 ;
	hsr_response_hdr(r) ;
	return(0) ;
    case HPM_SILENT:
	hsr_osend(r,HPM_CREATE,0) ;
	hsr_osend(r,HPM_INTERNAL,0) ;
	hsr_osend(r,HPM_REQUEST_DONE,0) ;
	break ;
    default:
	break ;
	}
    return(hpf_null(r,m,a)) ;
}

extern u32 hpf_nocontent_ok(HSR *r,int m,u32 a)
{
    switch (m) {
    case HPM_HEADER_SEND:
	r->http.status = 204 ;
	hsr_response_hdr(r) ;
	return(0) ;
	}
    return(hpf_generic_ok(r,m,a)) ;
}

extern u32 hpf_null(HSR *r,int m,u32 a)
{
    switch (m) {
    case HPM_GET_DESC:
	return((u32) r->hpo->desc) ;
    case HPM_GET_HELP:
	return((u32) r->hpo->help) ;
    case HPM_GET_NAME:
	return((u32) r->hpo->name) ;
    case HPM_URL_MATCH:
	return(url_match_root(r,(char *) hsr_osend(r,HPM_GET_NAME,a))) ;
    case HPM_HEADER_SEND:
	return(0) ;
    case HPM_CONTENT_SEND:
	return(0) ;
    case HPM_CAN_VISIT:
	return(1) ;
    case HPM_REQUEST_DONE:
	hsr_close(r) ;
	return 0 ;
    default:
	return(0) ;
	}
}

/* ================================================================ */
extern int *hpoa_initvec[] ;

typedef void (*HPOMAPFUN)(HSR *,u32) ;
typedef int (*HPOSCANFUN)(HSR *,u32) ;

extern void hpo_map(HPOMAPFUN fun,HSR *r,u32 a)
{
    HPO ***ii ;
    for (ii = (HPO ***) hpoa_initvec ; *ii ; ii++) {
	HPO **i ;
	for (i = *ii ; *i ; i++) {
	    hsr_obind(r,*i) ;
	    fun(r,a) ;
	    }
	}
    }

extern int hpo_scan(HPOSCANFUN fun,HSR *r,u32 a)
{
    u32 v ;
    HPO ***ii ;
    for (ii = (HPO ***) hpoa_initvec ; *ii ; ii++) {
	HPO **i ;
	for (i = *ii ; *i ; i++) {
	    hsr_obind(r,*i) ;
	    if (v = fun(r,a)) return(v) ;
	    }
	}
    return 0 ;
    }

/* ================================================================ */
#define TLF LF0001
#define TLC char

static u32 TLF(HSR *r,TLC *a)
{
    u32 v = hsr_osend(r,HPM_URL_MATCH,(u32) a) ;
    if (v) return (u32) r->hpo ;
    return 0 ;
}

extern HPO *hsr_bind_by_url(HSR *r)
{
    HPO *hpo ;
    if (!(hpo = (HPO *) hpo_scan((HPOSCANFUN) TLF,r,(u32) r->url->s)))
	hpo = hpo_not_found ;
    hsr_obind(r,hpo) ;
    return hpo ;
    }
    
#undef TLC
#undef TLF

extern HPO *hsr_bind_url(HSR *r,char *url)
{
    r->url->s = r->url->c = url ;
    r->url->e = url+strlen(url) ;
    return hsr_bind_by_url(r) ;
    }
    
/* ================================================================ */
#define TLF LF0002
#define TLA LA0002

typedef struct {
    int		m ;
    u32		a ;
} LA0002 ;

u32 TLF(HSR *r,TLA *a) {
    return hsr_osend(r,a->m,a->a) ;
}

static void hpo_send_all(HSR *r,int m,u32 a)
{
 TLA la[1] ;

    la->m = m ;
    la->a = a ;
    hpo_map((HPOMAPFUN) TLF,r,(u32) la) ;
    }

#undef TLF

static void hpo_init_all(void)
{
   HSR r[1] ;
   hpo_send_all(r,HPM_STATIC_CREATE,0) ;
   }

static void hpo_exit_all(void)
{
   HSR r[1] ;
   hpo_send_all(r,HPM_STATIC_DESTROY,0) ;
   }

/* ================================================================ */
LOGSEC lc_http[] = {"HTTP",1} ;

static void hsr_response(HSR *r)
{
    static char *ms[] = {"GET","POST","HEAD"} ;
    if (r->method == HM_HEAD) {
	r->http.status = 200 ;
	hsr_response_hdr(r) ;
	return ;
	}
    if (r->method == HM_GET || r->method == HM_POST) {
	log_printf(lc_http,"%s %s\n",ms[r->method],r->url->s) ;
	hsr_bind_by_url(r) ;
	hsr_osend(r,HPM_HEADER_SEND,0) ;
	hsr_osend(r,HPM_CREATE,0) ;
	hsr_osend(r,HPM_INTERNAL,0) ;
	hsr_osend(r,HPM_CONTENT_SEND,0) ;
	hsr_osend(r,HPM_REQUEST_DONE,0) ;
	}
}

#include	<pu/number.h>

static void unescape_in_place(char *s)
{
    char *i = s ;
    char *o = s ;
    if (!s) return ;
    while (*i) {
	if (*i == '%') {
	    int n ;
	    *o = 0 ;
	    if (!*++i || (n = digit(*i)) == DIGIT_INVALID) break ;
	    *o = n << 4 ;
	    if (!*++i || (n = digit(*i)) == DIGIT_INVALID) break ;
	    *o |= n ;
	    o++ ;
	    i++ ;
	    }
	else *o++ = *i++ ;
	}
    *o = 0 ;
    }

/* ================================================================ */
static void hsr_qs_destroy(HSR *r)
{
    if (r->qs.p) free(r->qs.p) ;
    if (r->qs.mt->s) mtfree(r->qs.mt) ;
    }

static void hsr_qs_parse(HSR *r)
{
    int		n ;
    char	*p,*s ;

    if (MTFillSize(r->args) == 0) return ;

    r->qs.mt[0] = r->args[0] ;
    mtstrdup_fill_pad(r->qs.mt,1) ;
    r->qs.mt->c[0] = 0 ;
    for (n = 1,p = r->qs.mt->s ; *p ; p++) {
	if (*p == '&') n++ ;
	}
    r->qs.n = n ;
    r->qs.p = tscalloc(n,QSE) ;
    s = r->qs.mt->s ;
    for (n = 0,p = r->qs.mt->s ; ; p++) {
	if (*p == '=') {
	    r->qs.p[n].name = s ;
	    s = p+1 ;
	    *p = 0 ;
	    }
	else if (*p == '&' || *p == 0) {
	    if (r->qs.p[n].name)
		 r->qs.p[n].value = s ;
	    else r->qs.p[n].name = s ;
	    s = p+1 ;
	    n++ ;
	    if (!*p) break ;
	    *p = 0 ;
	    }
	}
    for (n = 0 ; n < r->qs.n ; n++) {
	unescape_in_place(r->qs.p[n].value) ;
	}
    }

extern char *hsr_arg_exists(HSR *r,char *name)
{
    int i ;
    for (i = 0 ; i < r->qs.n ; i++) {
	if (!strcmp(r->qs.p[i].name,name))
	    return r->qs.p[i].value ;
	}
    return(0) ;
    }

extern char *hsr_arg_or_default(HSR *r,char *name,char *def)
{
    char	*s ;
    if (s = hsr_arg_exists(r,name)) return(s) ;
    return def ;
    }

extern char *hsr_arg_or_toggle(HSR *r,char *name)
{
    return hsr_arg_or_default(r,name,"toggle") ;
    }

extern char *hsr_arg_int(HSR *r,char *name,int *v)
{
    char	*a ;
    char	*e ;
    int		n ;
    if ((a = hsr_arg_exists(r,name))) return(0) ;
    if (!v) v = &n ;
    *v = strtol(name,&e,10) ;
    if (*e) return(0) ;
    return(a) ;
    }

/* ================================================================ */
static HSR *hsr_alloc(void)
{
    HSR *r = (HSR *) calloc(1,sizeof(HSR)) ;
    r->lc = r->qlc ;
    r->content_type = "text/html" ;
    r->csock = -1 ;
    return(r) ;
    }

static void hsr_free(HSR *r)
{
    free(r) ;
    }

/* ================================================================ */
static void hsr_request_prepare(HSR *r)
{
    MT		rm[1] ;
    MT		line[1] ;
    MT		*url = r->url ;
    MT		*args = r->args ;
    int n ;
    int x ;

    rm->c = rm->s = r->request->s ;
    rm->e = r->request->c ;

    *line = *rm ;
    mt_set_line(line) ;
    n = MTAllSize(line) ;

    MTSET(url,0,0) ;
    MTSET(args,0,0) ;

    if ((x = re_search(g_hsg.geturl,line->s,n,0,-1)) != -1) {
	rereg *reg = g_hsg.geturl->reg+1 ;
	int	o = reg->start ;
	while (1) {
	    if (o == reg->end) {
		MTSET(url,line->s+reg->start,line->s+reg->end) ;
		break ;
		}
	    if (line->s[o] == '?') {
		MTSET(url,line->s+reg->start,line->s+o) ;
		o++ ;
		MTSET(args,line->s+o,line->s+reg->end) ;
		break ;
		}
	    o++ ;
	    }
	r->method = HM_GET ;
	}

    else if ((x = re_search(g_hsg.posturl,line->s,n,0,-1)) != -1) {
	rereg *reg = g_hsg.posturl->reg+1 ;
	r->method = HM_POST ;
	MTSET(url,line->s+reg->start,line->s+reg->end) ;
	}

    else {
	r->method = HM_HEAD ;
	}

    while (1) {
	line->c = line->e ;
	line->e = rm->e ;
	mtspn_n(line,"\r\n",2) ;
	mt_set_line(line) ;
	if (MTAllSize(line) == 0) break ;
	}

    if (r->method == HM_POST) {
	MTSET(args,line->c,rm->e) ;
	mtspn_n(args,"\r\n",2) ;
	args->s = args->c ;
	}

    mtstrdup_all(r->url) ;
    mtstrdup_all(r->args) ;

    hsr_qs_parse(r) ;
    }

static void hsr_request_release(HSR *r)
{
    if (r->url->s)     mtfree(r->url) ;
    if (r->args->s)    mtfree(r->args) ;
    if (r->request->s) mtfree(r->request) ;

    hsr_qs_destroy(r) ;
    }

static void hsr_request(HSR *r)
{
    hsr_request_prepare(r) ;
    hsr_response(r) ;
    }

extern void hsr_close(HSR *r)
{
    hsr_osend(r,HPM_DESTROY,0) ;
    hsr_request_release(r) ;
    if (r->csock != -1) close(r->csock) ;
    hsr_free(r) ;
}


/* ================================================================ */
extern void hsr_execute(char *ss)
{
    HSR 	*r = hsr_alloc() ; ;
    MT		*url = r->url ;
    MT		*args = r->args ;
    char	*a ;
    char	*s = strdup(ss) ;
    if (a = strchr(s,'?')) {
	MTSETN(url,s,a-s) ;
	*a++ = 0 ;
	MTSETN(args,a,strlen(a)) ;
	args->c = args->e ;
	}
    else {
	MTSETN(url,s,strlen(s)) ;
	MTSETN(args,"",0) ;
	}
    mtstrdup_all(url) ;
    mtstrdup_all(args) ;
    hsr_qs_parse(r) ;
    hsr_bind_by_url(r) ;
    hsr_osend(r,HPM_SILENT,0) ;
    free(s) ;
    }

/* ================================================================ */
static int server_poll(HSS *hss)
{
    HSR 	*r = hsr_alloc() ; ;
    hsr_accept(hss,r) ;
    if (r->state & HSR_FAIL_MASK) goto fail ;

    /* ASSUMING here that the whole request will be read in
       better to create a UF that invokes the request once
       the complete request has been read in
    */

    hsr_request_read_loop(r) ;
    if (r->state & HSR_FAIL_MASK) goto fail ;

    hsr_request(r) ;
    return(0) ;
fail:
    if (r->csock != -1) close(r->csock) ;
    hsr_free(r) ;
    return -1 ;
}

static void hsg_exit() ;

static u32 httpd_uff(UF *uf,int m,u32 a)
{
    HSS *hss = (HSS *) uf->d.v ;
    switch(m) {
    case UFM_DESTROY:
	uf_socket_unregister(uf,hss->sockfd) ;
	close(hss->sockfd) ;
	uf->d.v = 0 ;
	hsg_exit() ;
	return(0) ;
    case UFM_TYPENAME:
	return(mtputs((MT *) a,"HTTP")) ;
    case UFM_SELECT_OK:
	server_poll((HSS *) uf->d.v) ;
	break ;
    default:
	break ;
	}
    return(null_uff(uf,m,a)) ;
    }

static int hss_socket_init(HSS *hss)
{
    UF *uf = uf_create(httpd_uff,hss,0) ;
    struct sockaddr_in serv_addr ;

    hss->sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (hss->sockfd < 0)  {
        hss_state_set(hss,HSS_SOCK_FAIL) ;
        return -1 ;
    }
{
    int flags = fcntl(hss->sockfd, F_GETFL, 0) ;
    fcntl(hss->sockfd, F_SETFL, flags | O_NONBLOCK) ;
    }
    
    bzero((char *) &serv_addr, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(hss->port);
 
    if (bind(hss->sockfd, (struct sockaddr *) &serv_addr,
                          sizeof(serv_addr)) < 0) {
         hss_state_set(hss,HSS_BIND_FAIL) ;
         return -1 ;
    }

    uf->d.v = hss ;
    uf_socket_register(uf,hss->sockfd) ;
    listen(hss->sockfd,5) ;

    return 0 ;
    }

static int hss_socket_open(HSS *hss)
{
    int		r ;
    if ((r = hss_socket_init(hss)) < 0) {
	errorshow("server_socket_init (%d) fail %08x",hss->port,hss->state) ;
	}
    return r ;
    }
    
/* ================================================================ */
static HSS *hss_alloc(void)
{
    return(tscalloc(1,HSS)) ;
    }

static void hss_free(HSS *hss)
{
    free(hss) ;
    }

extern HSS *hss_create(HSS_CREATE *c)
{
    HSS *hss = hss_alloc() ;
    hss->port = c->port ;
    hss_socket_open(hss) ;
    return(hss) ;
    }
    
extern HSS *hss_create_default(void)
{
    return(hss_create(hssc_default)) ;
    }
    
extern int hss_create_arg_unpack(HSS_CREATE *c,int argc,char **argv)
{
    return(argc) ;
    }
    
extern HSS *hss_create_args(int argc,char **argv)
{
    HSS_CREATE c[1] ;
    HSS *hss ;
    c[0] = hssc_default[0] ;
    hss_create_arg_unpack(c,argc,argv) ;
    hss = hss_create(c) ;
    return hss ;
    }
    
/* ================================================================ */
/* ~# use arg ; #~ */

/* ~~argset(help => "HTTP server settings")~~ */

/* ~~arg(help => "Set server document root to directory",value => "directory")~~ */
static int argf__http_document_root(char *name,char *value,void *a0)
{
    g_hsg.document_root = strdup(value) ;
    return(ASF_ARGACCEPTED) ;
}

/* ~~arg(help => "Set server port number",value => "port",type => "int")~~ */
static int argf__http_server_port(char *name,char *value,void *a0)
{
    hssc_default->port = strtol(value,0,10) ;
    return(ASF_ARGACCEPTED) ;
}

/* ~~arg(help => "Execute the command for the web page",value => "page")~~ */
static int argf__hexec(char *name,char *value,void *a0)
{
    if (g_hsg.runstate == 0) return ASF_DEFERRED ;
    hsr_execute(value) ;
    return(ASF_ARGACCEPTED) ;
}

/* ~~arg(help => "Install the HTTP server service",type => "command")~~ */
static int argf__http_service_install(char *name,char *value,void *a0)
{
    extern void InstallService(void) ;
    InstallService();
    return(ASF_ARGACCEPTED | ASF_VALUEIGNORED) ;
}

/* ~~arg(help => "Uninstall the HTTP server service",type => "command")~~ */
static int argf__http_service_uninstall(char *name,char *value,void *a0)
{
    extern void UninstallService(void) ;
    UninstallService();
    return(ASF_ARGACCEPTED | ASF_VALUEIGNORED) ;
}

/* ~~arg(help => "Start the HTTP server service",type => "command")~~ */
static int argf__http_service_start(char *name,char *value,void *a0)
{
    extern void RunService(void) ;
    RunService() ;
    return(ASF_ARGACCEPTED | ASF_VALUEIGNORED) ;
}

/* ================================================================ */
#include	<pu/regex.h>

/* problems revealed here
   this is a 'help' function which should disable all further processing
   but it only gets called after the http server is being started
   */

#define TLC LC0000
#define TLF LF0000

typedef struct {
    REGEX	*rx ;
} LC0000 ;

static void TLF(HSR *r,TLC *a)
{
    char *name ;
    char *desc ;
    if (!hsr_osend(r,HPM_CAN_VISIT,0)) return ;
    name = (char *) hsr_osend(r,HPM_GET_NAME,0) ;
    if (!name) return ;
    if (re_search(a->rx,name,strlen(name),0,-1) != -1) {
	desc = (char *) hsr_osend(r,HPM_GET_HELP,0) ;
	printf("%-24s",name) ;
	printf("%s\n",desc) ;
	fflush(stdout) ;
	}
    }

/* ~~arg(help => "Show names of pages and their effects",value => "PATTERN")~~ */
static int argf__http_show_pages(char *name,char *value,void *a0)
{
    REGEX	rx[1] ;
    memset(rx,0,sizeof(REGEX)) ;
    rx->regexp = 1 ;
    if (!*value) value = "." ;
    re_compile_pattern_s(rx,value) ;
{
    HSR r[1] ;
    TLC a[1] ;
    memset(r,0,sizeof(HSR)) ;
    memset(a,0,sizeof(TLC)) ;
    a->rx = rx ;
    hpo_map((HPOMAPFUN) TLF,r,(u32) a) ;
    }
    re_free(rx) ;
    exit(0) ;
    return(ASF_ARGACCEPTED) ;
}

#undef TLC
#undef TLF

/* ================================================================ */
/* this is not quite solid, choose which method */

static void hsg_exit()
{
    re_free(g_hsg.geturl) ;
    re_free(g_hsg.posturl) ;
    hpo_exit_all() ;
    }

extern void hsg_init_static(void)
{
    g_hsg.runstate = 1 ;
    g_hsg.geturl->regexp = 1 ;
    re_compile_pattern_s(g_hsg.geturl,"GET \\(.*\\) HTTP") ;
    g_hsg.posturl->regexp = 1 ;
    re_compile_pattern_s(g_hsg.posturl,"POST \\(.*\\) HTTP") ;
    }

/* ================================================================ */
static HSS *g_hss ;

static int http_arg_read(int argc,char **argv)
{
    return argset_try_one(argc,argv,argset_http) ;
}

extern void hsg_init(int argc,char **argv)
{
    hsg_init_static() ;
    argc = http_arg_read(argc,argv) ;
    g_hss = hss_create_args(argc,argv) ;
    hpo_init_all() ;
    }

#include	"atinit.h"

extern u32 http_atinit(int phase,u32 a)
{
    switch(phase) {
    case AT_MAIN_PRE_MODE:
	return(0) ;
	}
    return(0) ;
    }

/* ~# use atinit ; atinit::register() ; #~ */

/* ================================================================ */
static int http_main(int argc,char **argv,char *mode)
{
    int		i ;

    hsg_init_static() ;

    if ((i = arg_peek(argc,argv,"--no-init")) == -1) {
	arg_expand_try_file(&argc,&argv,".http") ;
	}
    else argv[i] = 0 ;

    argc = log_init(argc,argv) ;

    argc = http_arg_read(argc,argv) ;

    if (g_hsg.nostart) return(RC_OK) ;

    argc = ufs_init(argc,argv) ;
    g_hss = hss_create_args(argc,argv) ;

    if (!g_hss->sockfd)
	errorfatal("Server didn't start") ;
    else printf("Server started on port %d\n",g_hss->port) ;

    hpo_init_all() ;

    ufs_loop() ;

    ufs_destroy() ;

    return(RC_OK) ;
    }

/* ================================================================ */
/* ~# use mainmode ; #~ */
/* ~~mode("http",
   desc		=> "Run the HTTP server only",
   )~~ */

#include	".gen/http.c"


#ifdef __cplusplus /*Z*/
}
#endif
