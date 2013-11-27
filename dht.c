#ifdef __cplusplus /*Z*/
extern "C" {
#endif

#include	<unistd.h>
#include	<string.h>
#include	<errno.h>

#include	"pu/mt.h"

#include	"pu/rcons.h"
#include	"pu/regex.h"

#include	<sys/time.h>

#include	"log.h"
/* ~# use collect ; collect::register_all('logsec','^LOGSEC (\w+)','$1') ; #~ */

#include	"ma.h"
#include	"dht.h"

#include	"jsf.h"
#include	"uf.h"

#include	"tc_sql.h"

#define UF_ARG_WANT__UFM_ARG_DESCRIBE	1

#include	"uf_arg.h"

#include	"common.h"

#define HTTP_PORT	80
#define HTTPS_PORT	443

#define USERAGENT "Opera/9.80 (Windows NT 5.2; WOW64; U; en) Presto/2.10.229 Version/11.64"

static struct {
    struct {
	char	*cookie ;
	int	seq ;
	} auth ;
    struct {
	char	*s ;
	REGEX	r[1] ;
	} http_status_pat ;
    } g = {
    } ;

/* ================================================================ */
extern int header_find(MD *mi,MD *mo,char *name)
{
    char	*s,*c ;
    int namelen = strlen(name) ;
    s = c = mi->s ;
    while (1) {
	if (c >= mi->e) goto end ;
	switch (*c) {
	case '\r':
	    c++ ;
	    if (c > mi->e) goto end ;
	    if (*c == '\n') c++ ;
	    s = c ;
	    break ;
	case ':':
	    if (!memcmp(s,name,namelen)) {
		goto getval ;
		}
	    }
	c++ ;
	}
end:
    return 0 ;
getval:
    c++ ;
    while (*c == ' ') c++ ;
    mo->s = c ;
    while (1) {
	if (c >= mi->e) {
	    mo->e = mi->e ;
	    return 1 ;
	    }
	switch (*c) {
	case '\r':
	case '\n':
	    mo->e = c ;
	    return 1 ;
	}
	c++ ;
	}
    }

extern int header_copy(MD *mi,MT *mto,char *name)
{
    MD		mo[1] ;
    int		r ;
    if ((r = header_find(mi,mo,name)) != 0) {
	mtmcpy(mto,mo->s,mo->e-mo->s) ;
	*mto->c = 0 ;
	}
    return r ;
    }

extern int ht_header_find(HT *ht,MD *mo,char *name)
{
    MD		mi[1] ;
    mi->s = ht->response->s ;
    mi->e = mi->s + ht->rsp.obody_s ;
    return header_find(mi,mo,name) ;
    }

extern int ht_header_copy(HT *ht,MT *mto,char *name)
{
    MD		mo[1] ;
    int		r ;
    if ((r = ht_header_find(ht,mo,name)) != 0) {
	mtmcpy(mto,mo->s,mo->e-mo->s) ;
	*mto->c = 0 ;
	}
    return r ;
    }


/* ================================================================ */
extern void param_add(MT *mt,char *name,char *value)
{
    mtprintf(mt,"\"%s\":[\"%s\"]",name,value) ;
    }

extern void param_add_u(MT *mt,char *name,char *value)
{
    mtprintf(mt,"\"%s\":[%s]",name,value) ;
    }

/* ================================================================ */
static void tod_rcons_qlistbuild(MT *mt,RCONS *r)
{
    if (!r) return ;
    while (1) {
	TOD *t = (TOD *) r->car ;
	mtprintf(mt,"\"%s\"",t->hash) ;
	if (!(r = r->cdr)) break ;
	mtputc(mt,',') ;
	}
    }

static void buildargpair(MT *mt,char *name,char *format,u32 arg)
{
    mtprintf(mt,"\"%s\": ",name) ;
    mtprintf(mt,format,arg) ;
    }

/* ================================================================ */
/*
  select returns a bit set for each file descriptor that is ready to read.
  we have a set of structures that don't neccessarily all have file descriptors
  Those that do can be scheduled with select, so no polling of filehandles
  is required.
  We don't want to have to poll each of the other entities which have subtasks
  which we do not want to block the main thread.
  
*/
/* ================================================================ */
static void ht_socket_register(HT *ht)
{
    uf_socket_register(ht->uf,ht->sock) ;
    }

static void ht_socket_unregister(HT *ht)
{
    uf_socket_unregister(ht->uf,ht->sock) ;
}

/* ================================================================ */
#include	<sys/socket.h>
#include	<netdb.h>

LOGSEC ht_data_send[] = {"HT_DATA_SEND",0} ;
LOGSEC ht_data_rcv[] = {"HT_DATA_RCV",1} ;

static int ht_recv(HT *ht,void *buf, size_t len) ;
static ssize_t ht_send(HT *ht, void *buf, size_t len) ;

static void ht_error(HT *ht,char *reason)
{
    errorshow("HT fail: %s",reason) ;
    ht->error = reason ;
}

static void ht_socket_create(HT *ht)
{
    ht->sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP) ;
    if (ht->sock) {
	HT_SETDONE(ht,HTS_SOCK) ;
	ht_socket_register(ht) ;
	}
    else {
	HT_SETFAIL(ht,HTS_SOCK) ;
	}
}

static void ht_get_ip(HT *ht)
{
    struct hostent *hent;

    if ((hent = gethostbyname(ht->host)) == NULL) {
	HT_SETFAIL(ht,HTS_GET_IP) ;
	return ;
	}

    ht->ipaddr = *((struct in_addr *) hent->h_addr_list[0]) ;
    HT_SETDONE(ht,HTS_GET_IP) ;
}
 
static int ht_request_send(HT *ht)
{
    int cbsend ;
    MT	*mt = ht->request ;
    char	*s = mt->s ;

    log_puts_escape_nl(ht_data_send,s) ;
    log_puts_c(ht_data_send,"\n") ;

    while (s < mt->c) { 
	cbsend = ht_send(ht, s, mt->c - s);
	if (cbsend == -1) {
	    HT_SETFAIL(ht,HTS_REQUEST) ;
	    return -1 ;
	}
      s += cbsend ;
    }	
    HT_SETDONE(ht,HTS_REQUEST) ;
    return 0 ;
}

static int http_status_read(char *s,char *e)
{
    MT		mt[1] ;
    REGEX *r = g.http_status_pat.r ;
    int n = e-s ;
    int x = re_search(r,s,n,0,n) ;
    if (x == -1) { return -1 ;}
    MTALLOCA(mt,n) ;
    mtput(mt,s+r->reg[1].start,r->reg[1].end - r->reg[1].start) ;
    mtputc(mt,0) ;
    return(strtol(mt->s,0,10)) ;
    }
   
static void ht_response_headers(UF *uf,HT *ht,char *s,char *e)
{
    MT		mt[1] ;
    MTSET(mt,s,e) ;
    ht->http_status = http_status_read(s,e) ;
    if (ht->http_status > 400) {
	errorshow("HTTP STATUS: seq=%d status=%d\n",ht->seq,ht->http_status) ;
	}
    uf_send(uf,UFM_HT_RESPONSE_HDR,(u32) mt) ;
    }

extern int GZRead_mt(MT *,MT *) ;

extern void UFM_HT_RESPONSE_JSON_arg_desc(MT *mt,u32 a)
{
    RBP *p = (RBP *) a ;
    mtcpymt_all(mt,p->mtu) ;
    }

static void ht_response_body(UF *uf,char *s,char *e)
{
    }

static void send_response_body(UF *uf,RBP *p)
{
//    log_puts_escape_nl(ht_data_rcv,p->mtu->s) ;
//    log_puts_c(ht_data_rcv,"\n") ;
    uf_send(uf,UFM_HT_RESPONSE_BODY,(u32) p) ;
    }

static void ht_response_body_complete(UF *uf,HT *ht)
{
    RBP p[1] ;
    p->mtz->c = p->mtz->s = ht->response->s + ht->rsp.obody_s ;
    p->mtz->e = ht->response->c ;
{
    MT		mc[1] ;
    MTALLOCA(mc,1024) ;
    if (ht_header_copy(ht,mc,"Content-Encoding")) {
	if (!strcmp(mc->s,"gzip")) {
	    mtmalloc(p->mtu,65536) ;
	    GZRead_mt(p->mtz,p->mtu) ;
	    *(p->mtu->c) = 0 ;
	    send_response_body(uf,p) ;
	    free(p->mtu->s) ;
	    return ;
	}
    }
   p->mtu[0] = p->mtz[0] ;	/* uncompressed == compressed */
   send_response_body(uf,p) ;
 }
}

static void ht_response_read(UF *uf,HT *ht)
{
    int		n ;
    int		slice = 65536 ;
    MT		*mt = ht->response ;

    if (mt->c + slice > mt->e) {
	int size = mt->s ? MTAllSize(mt) * 4 : slice * 4 ;
	mtrealloc(mt,size) ;
    }
    n = ht_recv(ht,mt->c,slice) ;
    if (n < 0) {
	return ;
    }
    if (n == 0) {
	ht_response_body_complete(uf,ht) ;
	HT_SETDONE(ht,HTS_RESPONSE) ;
	return ;
    }
    *(mt->c += n) = 0 ;
    if (ht->rsp.obody_s == -1) {
	char *hend ;
	if (hend = strstr(mt->s,"\r\n\r\n")) {
	    int	ohend = hend - mt->s ;
	    ht->rsp.obody_s = ohend + 4 ;
	    ht->rsp.obody_c = ht->rsp.obody_s ;
	    ht_response_headers(uf,ht,mt->s,mt->s+ohend) ;
	    ht_response_body(uf,mt->s + ht->rsp.obody_c,mt->c) ;
	    ht->rsp.obody_c = mt->c - mt->s ;
	}
	else ; 		/* don't get cute with this logic */
    }
    else {
	ht_response_body(uf,mt->s + ht->rsp.obody_c,mt->c) ;
	ht->rsp.obody_c = mt->c - mt->s ;
    }
}

static void ht_request_build(HT *ht)
{
    MT	*mt = ht->request ;
    mtmalloc(mt,4096) ;

    mtprintf(mt,"%s /%s HTTP/1.0\r\n",ht->method,ht->page) ;
    mtprintf(mt,"User-Agent: %s\r\n",USERAGENT) ;
    mtprintf(mt,"Host: %s\r\n",ht->host) ;
//    mtprintf(mt,"Accept-Encoding: gzip;q=0\r\n") ;
    mtprintf(mt,"Accept-Encoding: */*\r\n") ;
    if (ht->cookie) 
	mtprintf(mt,"Cookie: %s\r\n",ht->cookie) ;
    if (ht->post) {
	mtprintf(mt,"Content-Length: %d\r\n",strlen(ht->post)) ;
	mtprintf(mt,"Content-Type: application/x-www-form-urlencoded\r\n") ;
	}
    mtprintf(mt,"\r\n") ;
    if (ht->post) {
	mtputs(mt,ht->post) ;
	}
    }

static void ht_free(HT *ht)
{
    free(ht) ;
    }

static HT *ht_alloc(void)
{
    HT *ht = (HT *) calloc(1,sizeof(HT)) ;
    return ht ;
    }

static void ht_connect(HT *ht)
{
    struct sockaddr_in remote[1] ;
    int		retry = 0 ;
    remote->sin_family = AF_INET;
    remote->sin_addr = ht->ipaddr ;
    remote->sin_port = htons(ht->port) ;

    while (1) {
	if (connect(ht->sock, (struct sockaddr *)remote, sizeof(struct sockaddr)) == 0)
	    break ;
	perror("connect error") ;
	if (++retry == 3) {
	    HT_SETFAIL(ht,HTS_CONNECT) ;
	    return ;
	    }
    }
    if (ht->ssl) {
	}
    else {
	HT_SETDONE(ht,HTS_SSL_CONNECT) ;
	}
    HT_SETDONE(ht,HTS_CONNECT) ;
    }

static void htssl_destroy(SSLHT *) ;

static void ht_destroy(HT *ht)
{
    if (ht->request->s) free(ht->request->s) ;
    if (ht->response->s) free(ht->response->s) ;
    if (ht->sock) {
	close(ht->sock) ;
	ht_socket_unregister(ht) ;
	}
    if (ht->host) free(ht->host) ;
    if (ht->post) free(ht->post) ;
    if (ht->page) free(ht->page) ;
    if (ht->method) free(ht->method) ;
    if (ht->ssl) htssl_destroy(ht->ssl) ;
    }

extern HT *ht_create_port(int port,char *method,char *host,char *page,char *post)
{
    HT *ht ;
    ht = ht_alloc() ;
    ht->method = strdup(method) ;
    ht->host = strdup(host) ;
    ht->page = strdup(page) ;
    if (post) 
	 ht->post = strdup(post) ;
    else ht->post = 0 ;
    ht->rsp.obody_s = -1 ;
    ht->rsp.obody_c = 0 ;
    ht->port = port ;
    return(ht) ;
}

extern HT *ht_create(char *method,char *host,char *page,char *post)
{
    return(ht_create_port(HTTP_PORT,method,host,page,post)) ;
}

/* ================================================================ */
#include	"sock.h"

#define CSELECT_IN   0x01
#define CSELECT_OUT  0x02
#define CSELECT_ERR  0x04

extern int select_rw(int readfd, int writefd, int timeout_ms)
{
  struct timeval timeout;
  fd_set fds_read;
  fd_set fds_write;
  fd_set fds_err;
  socket_t maxfd;
  int r;
  int ret;

  timeout.tv_sec = timeout_ms / 1000;
  timeout.tv_usec = (timeout_ms % 1000) * 1000;

  FD_ZERO(&fds_err);
  maxfd = -1;

  FD_ZERO(&fds_read);
  if (readfd != SOCKET_BAD) {
    VERIFY_SOCK(readfd);
    FD_SET(readfd, &fds_read);
    FD_SET(readfd, &fds_err);
    maxfd = readfd;
  }

  FD_ZERO(&fds_write);
  if (writefd != SOCKET_BAD) {
    VERIFY_SOCK(writefd);
    FD_SET(writefd, &fds_write);
    FD_SET(writefd, &fds_err);
    if (writefd > maxfd)
      maxfd = writefd;
  }

  do {
    r = select((int)maxfd + 1, &fds_read, &fds_write, &fds_err, &timeout);
  } while((r == -1) && (socket_errno() == EINTR));

  if (r < 0)
    return -1;
  if (r == 0)
    return 0;

  ret = 0;
  if (readfd != SOCKET_BAD) {
    if (FD_ISSET(readfd, &fds_read))
      ret |= CSELECT_IN;
    if (FD_ISSET(readfd, &fds_err))
      ret |= CSELECT_ERR;
  }
  if (writefd != SOCKET_BAD) {
    if (FD_ISSET(writefd, &fds_write))
      ret |= CSELECT_OUT;
    if (FD_ISSET(writefd, &fds_err))
      ret |= CSELECT_ERR;
  }

  return ret;
}

/* ================================================================ */
#include	<openssl/ssl.h>

/* enum for the nonblocking SSL connection state machine */
typedef enum {
  ssl_connect_1,
  ssl_connect_2,
  ssl_connect_2_reading,
  ssl_connect_2_writing,
  ssl_connect_3,
  ssl_connect_done
} ssl_connect_state;

struct struct_SSLHT {
    u32		state ;
    SSL_CTX	*ctx ;
    SSL		*handle ;
    ssl_connect_state connecting_state;
    struct {
	int	verifypeer ;
	} opt ;
    } ;

static void htssl_destroy(SSLHT *ssl)
{
    free(ssl) ;
    }

/*
ssluse.c:1187:    req_method = SSLv23_client_method();

callbacks to ssl_tls_trace (write debug code) show same sequence
as in gdb-130830-175753.out
what we are not doing correctly is emulating what Curl_ossl_connect_step2
does

SSL_write(ssl->handle, mem, (int)len);
ssize_t nread = (ssize_t)SSL_read(ssl->handle, buf, (int)buffersize);
*/

#if 0
#define SSL_ERROR_NONE			0
#define SSL_ERROR_SSL			1
#define SSL_ERROR_WANT_READ		2
#define SSL_ERROR_WANT_WRITE		3
#define SSL_ERROR_WANT_X509_LOOKUP	4
#define SSL_ERROR_SYSCALL		5
#define SSL_ERROR_ZERO_RETURN		6
#define SSL_ERROR_WANT_CONNECT		7
#define SSL_ERROR_WANT_ACCEPT		8
#endif

/* ~# use decode ; #~ */
/* ~~define_decode("SSL_\\w+")~~ */

extern const char *ssl_msg_type2(int m)
{
    switch (m) {
    case SSL2_MT_ERROR:        			return "Error";
    case SSL2_MT_CLIENT_HELLO: 			return "Client hello";
    case SSL2_MT_CLIENT_MASTER_KEY: 		return "Client key";
    case SSL2_MT_CLIENT_FINISHED: 		return "Client finished";
    case SSL2_MT_SERVER_HELLO: 			return "Server hello";
    case SSL2_MT_SERVER_VERIFY: 		return "Server verify";
    case SSL2_MT_SERVER_FINISHED: 		return "Server finished";
    case SSL2_MT_REQUEST_CERTIFICATE: 		return "Request CERT";
    case SSL2_MT_CLIENT_CERTIFICATE: 		return "Client CERT";
    default:					return "*unknown*" ;
    }
    }

extern const char *ssl_msg_type3(int m)
{
    switch (m) {
    case SSL3_MT_HELLO_REQUEST: 		return "Hello request";
    case SSL3_MT_CLIENT_HELLO: 			return "Client hello";
    case SSL3_MT_SERVER_HELLO: 			return "Server hello";
    case SSL3_MT_NEWSESSION_TICKET:		return "New Session ticket" ;
    case SSL3_MT_CERTIFICATE: 			return "CERT";
    case SSL3_MT_SERVER_KEY_EXCHANGE: 		return "Server key exchange";
    case SSL3_MT_CLIENT_KEY_EXCHANGE: 		return "Client key exchange";
    case SSL3_MT_CERTIFICATE_REQUEST: 		return "Request CERT";
    case SSL3_MT_SERVER_DONE: 			return "Server finished";
    case SSL3_MT_CERTIFICATE_VERIFY: 		return "CERT verify";
    case SSL3_MT_FINISHED: 			return "Finished";
    default:					return "*unknown*" ;
    }
    }

extern const char *tls_rt_type(int type)
{
    switch (type) {
    case SSL3_RT_CHANGE_CIPHER_SPEC:		return "change cipher" ;
    case SSL3_RT_ALERT: 			return "alert" ;
    case SSL3_RT_HANDSHAKE:			return "handshake" ; 
    case SSL3_RT_APPLICATION_DATA: 		return "app data" ;
    default:					return "*unknown*" ;
}
}


LOGSEC ssl_ls[] = {"SSL",1} ;

static void ssl_tls_trace(int direction, int ssl_ver, int content_type,
                          char *buf, size_t len, const SSL *ssl,
                          SSLHT	*sslht)
{
    int vm = ssl_ver >> 8;
    MT		mt[1] ;
    MTALLOCA(mt,1024) ;
    mtprintf(mt,"%s ",direction ? "  >" : "<  ") ;
    if (vm == SSL2_VERSION_MAJOR) {
	mtprintf(mt,"v2 msg:%d (%s) ",buf[0],ssl_msg_type2(buf[0])) ;
	}
    else if (vm == SSL3_VERSION_MAJOR) {
	mtprintf(mt,"v3 TLS:%d (%s) msg:%d (%s)",content_type,tls_rt_type(content_type),buf[0],ssl_msg_type3(buf[0])) ;
	}
    else {
	mtprintf(mt,"v%d (unrecognized ssl version)",vm) ;
	}
    log_printf(ssl_ls,"%s\n",mt->s) ;
}

static void set_callbacks(SSLHT	*ssl)
{
    if (!SSL_CTX_callback_ctrl(ssl->ctx,
			       SSL_CTRL_SET_MSG_CALLBACK,
			       (void (*)(void)) ssl_tls_trace)) {
	errorshow("SSL: couldn't set callback argument!\n");
	return ;
    }
    if (!SSL_CTX_ctrl(ssl->ctx, 
		      SSL_CTRL_SET_MSG_CALLBACK_ARG, 0,
		      ssl)) {
	errorshow("SSL: couldn't set callback argument!\n");
    }
    }

static int cert_verify_callback(int ok, X509_STORE_CTX *ctx)
{
    return 0 ;
    }

#include	"glt.h"

GLT glt_SSL_[] ;

static void ht_ssl_connect(HT *ht)
{
    SSLHT	*ssl = ht->ssl ;
    const SSL_METHOD *req_method ;
{
    static int i ;
    if (i == 0) {
	SSL_library_init() ;	/* no error code */
	i = 1 ;
	}
    }
    req_method = SSLv23_client_method();
    ssl->ctx = SSL_CTX_new(req_method) ;
    set_callbacks(ssl) ;
    SSL_CTX_set_options(ssl->ctx, SSL_OP_ALL);

    SSL_CTX_set_verify(ssl->ctx,
                     ssl->opt.verifypeer?SSL_VERIFY_PEER:SSL_VERIFY_NONE,
                     cert_verify_callback) ;

    /* callback */
    ssl->handle = SSL_new(ssl->ctx);
    SSL_set_connect_state(ssl->handle);
    if (!SSL_set_fd(ssl->handle, ht->sock)) {
	errorshow("SSL_set_fd fail\n") ;
	}
{
    int		err ;
    int detail ;
    while (1) {
	err = SSL_connect(ssl->handle) ;
	if (err == 1) {
	    log_printf(ssl_ls,"SSL_connect returns %d\n",err) ;
	    HT_SETDONE(ht,HTS_SSL_CONNECT) ;
	    break ;
	    }
	detail = SSL_get_error(ssl->handle, err) ;
	log_printf(ssl_ls,"SSL_connect returns %d - detail %d (%s)",err,detail,gltget_c(glt_SSL_,detail)) ;
	}
    }
    }

extern HT *https_create(char *method,char *host,char *page,char *post)
{
    HT *ht = ht_create_port(HTTPS_PORT,method,host,page,post) ;
    ht->ssl = tscalloc(1,SSLHT) ;
    return(ht) ;
}

/* ================================================================ */
static int ht_recv(HT *ht,void *buf, size_t len)
{
    int		n ;
    if (ht->ssl) {
	if (!ht->ssl->handle) {
	    return -1 ;
	    }
	n = SSL_read(ht->ssl->handle, buf,len) ;
	}
    else {
	n = recv(ht->sock, buf, len, 0) ;
	if (n < 0) {
	    int e = errno ;
	    if (e != 107) {
		HT_SETFAIL(ht,HTS_RESPONSE) ;
		perror("recv error") ;
	    }
	}
    }
    return n ;
    }

static ssize_t ht_send(HT *ht, void *buf, size_t len)
{
    int		n ;
    if (ht->ssl) {
	n = SSL_write(ht->ssl->handle, buf,len) ;
	}
    else {
	n = send(ht->sock,buf,len,0);
	}
    return n ;
    }
    

/* ================================================================ */
extern UF *deluge_ht_create(UFF uff,char *post,void *cp)
{
    HT		*ht ;
    UF		*uf ;
    MT		mtpost[1] ;
    MTALLOCA(mtpost,strlen(post)+100) ;
    mtprintf(mtpost,post,g.auth.seq) ;
    ht = ht_create("POST","erebus.feralhosting.com","earwig/deluge/json",mtpost->s) ;
    uf = uf_create(uff,ht,cp) ;
    uf->d.ht->seq = g.auth.seq++ ;
    return uf ;
    }

extern UF *tod_deluge_ht_create(UFF uff,char *post,TOD *tod)
{
    UF *uf = deluge_ht_create(uff,post,tod) ;
    return(uf) ;
    }

/* ================================================================ */
static int ht_action(UF *uf,HT *ht)
{
    int		state = ht->state ;
    if (HT_ISNEED(ht,HTS_SOCK)) {
	ht_socket_create(ht) ;
	goto done ;
	}
    if (HT_ISNEED(ht,HTS_GET_IP)) {
	ht_get_ip(ht) ;
	goto done ;
	}
    if (HT_ISNEED(ht,HTS_CONNECT)) {
	stimer_block() ;
	ht_connect(ht) ;
	stimer_unblock() ;
	goto done ;
	}
    if (HT_ISNEED(ht,HTS_SSL_CONNECT)) {
	ht_ssl_connect(ht) ;
	goto done ;
	}
    if (HT_ISNEED(ht,HTS_REQUEST)) {
	if (!(ht->cookie)) ht->cookie = g.auth.cookie ;
	ht_request_build(ht) ;
	ht_request_send(ht) ;
	goto done ;
	}
done:
    return(state != ht->state) ;
}

static void ht_state_check(UF *uf,HT *ht)
{
    if (ht->fail) {
	log_printg(ls_generic,"%{uf}s failure %d\n",uf,ht->fail) ;
	printf("fail 0x%04x\n",ht->fail) ;
	uf->state |= UFS_DESTROY ;
	}
    if (!HT_ISNEED(ht,HTS_RESPONSE)) {
	uf->state |= UFS_DESTROY ;
	}
}

static int ufm_arg_describe(UF *f,int m,u32 a,MT *mt)
{
    switch(m) {
    case UFM_HT_RESPONSE_JSON:
	UFM_HT_RESPONSE_JSON_arg_desc(mt,a) ;
	return(1) ;

    case UFM_HT_RESPONSE_HDR:
	mtcpymt_to_first_nl(mt,(MT *) a) ;
	return(1) ;
    }
    return(0) ;
    }

/* ================================================================ */
extern void mt_dump_f(MT *mt,FILE *f) ;

/* ~# collect::register('uff','ht_uff') ; #~ */

UFSS ht_ufss[] = {"HT"} ;

extern u32 ht_uff(UF *uf,int m,u32 a)
{
    HT		*ht = uf->d.ht ;
    int		r ;
    switch(m) {
    case UFM_GET_STATIC:
	return (u32) ht_ufss ;
    case UFM_ARG_DESCRIBE:			/*O*/
	if (UFM_ARG_DESCRIBE_apply(ufm_arg_describe,uf,a)) return(1) ;
	break ;
    case UFM_CREATE:				/*A*/
	ht->uf = uf ;
	ht->sc = (void *) a ;
	break ;
    case UFM_DESTROY:				/*A*/
	ht_destroy(ht) ;
	ht_free(ht) ;
	break ;
    case UFM_STATE_REPORT:
	if (uf_send_direct(uf,UFM_TRACE_OK,m)) {
	    printf("action: %lu state: %08x fail: %08x\n",a,ht->state,ht->fail) ;
	    }
	return(0) ;
    case UFM_ACTION:				/*A*/
	if (uf_send(uf,UFM_HT_NOTREADY,0)) return 0 ;
	r = ht_action(uf,ht) ;
	ht_state_check(uf,ht) ;
	return(r) ;
    case UFM_SELECT_OK:				/*A*/
	ht_response_read(uf,ht) ;
	return(0) ;

    case UFM_HT_TORRENT_STATUS_GET:		/*B*/
    case UFM_HT_RESPONSE_JSON:			/*B*/
	/* messages that should have been handled in the subclass */
	return(0) ;
    default:
	break ;
	}
    return(null_uff(uf,m,a)) ;
    }

/* subclasses of ht_uff:
   A) must		pass message to ht_uff after local processing
   B) should not	pass message to ht_uff after local processing
   O) can do what they want
*/

extern u32 dht_uff(UF *uf,int m,u32 a)
{
    switch(m) {
    case UFM_HT_NOTREADY:
	return(!g.auth.cookie) ;
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
static void tod_tab_populate_db(void)
{
}

/* ================================================================ */
static void ht_auth_uff_response(UF *uf,HT *ht)
{
    MD		m[1] ;
    MD		mo[1] ;
    m->s = ht->response->s ;
    m->e = m->s + ht->rsp.obody_s ;
    if (header_find(m,mo,"Set-Cookie")) {
	char	*e = mo->s ;
	while (*e != ';') e++ ;
	g.auth.cookie = memtocstring(mo->s,e-mo->s) ;
	}
    }
    

static u32 ht_auth_uff(UF *uf,int m,u32 a)
{
    HT		*ht = uf->d.ht ;
    switch(m) {
    case UFM_HT_NOTREADY:
	return(0) ;
    case UFM_TYPENAME:	return(mtputs((MT *) a,"AUTH")) ;
    case UFM_HT_RESPONSE_HDR:
	ht_auth_uff_response(uf,ht) ;
	break ;
    case UFM_HT_RESPONSE_JSON:
	break ;
    default:
	break ;
	}
    return(dht_uff(uf,m,a)) ;
    }

static void dht_auth(void)
{
    MT		mt[1] ;
    UF		*uf ;
    char *t = "{\"method\":\"auth.login\",\
\"params\":[\"J-TJfn6iDZ-M6uJp\"],\
\"id\":%%d\
}\
" ;
    MTALLOCA(mt,DHT_MAX) ;
    mtprintf(mt,t) ;
    uf = deluge_ht_create(ht_auth_uff,mt->s,0) ;
    ufs_rc_add(uf) ;
    }
    
/* ================================================================ */
/* in this case we have freedom to name our structure fields the same
   as the JSON names, and also the database column names.
   Where a translation from json name to structure name or database
   column name is enforced (because of legacy for example)
   we would have a lookup indexed by the SFI column
   */

/* TSI ---> Torrent Status Info */
/* THSI ---> Torrent Hash Status Info */
/* TLSI ---> Torrent List Status Info */

/* ================================================================ */
extern void tsi_unpack(TSI *tsi,json_t *value) ;
extern void tsi_destroy(TSI *tsi) ;

static void tlsi_response_itf(const char *hash,json_t *value,void *a)
{
    TOD *tod = tod_intern((char *) hash) ;
    tsi_unpack(tod->tsi_c,value) ;
    uf_send((UF *) a,UFM_HT_TORRENT_STATUS_GET,(u32) tod) ;
    }

extern void tlsi_accept_response(UF *uf,HT *ht,RBP *r)
{
    json_t *ej = json_object_get(r->j,"error") ;
    json_t *rj = json_object_get(r->j,"result") ;
    const char *es = json_string_value(ej) ;
    const char *rs = json_string_value(rj) ;
    (void) es ;
    (void) rs ;
    json_object_iterate(rj,tlsi_response_itf,uf) ;
}

/* ================================================================ */
/* THMC ---> Torrent Hash Method Control */

typedef struct struct_THMC THMC ;

typedef u32 (*THMF)(THMC *,int,u32) ;

struct struct_THMC {
    THMF	f ;
    char	*method ;
    } ;

static void thmc_destroy_and_free(THMC *c)
{
    free(c) ;
    }

/* ================================================================ */
/* THCP ---> torrent health check poll */

typedef struct timeval TIME ;

typedef struct {
    int		running ;
    STIMER	*st ;
    TIME	time[1] ;
    RCONS	*refresh ;
    } TSG ;

static TSG tsg_g ;

#include	<pu/mathfuns.h>
#include	<pu/util.h>

static int check_period(TOD *t,TSI *i)
{
    if (i->state != downloading) return 0 ;
    if (i->progress > 90) return 0 ;
    if (i->download_payload_rate > 100000) {
	return 1 ;
	}
{
    int64 remain = i->total_size - i->total_done ;
    int		log ;
    log = ilog2(tsg_g.time->tv_sec - i->time_added + 1) ;

    if (i->progress < 0.1)
 	 log = max(1,min(3,log - 6)) ;
    else log = max(1,min(8,log - 5)) ;

    (void) remain ;
    return (1 << log) ;
    }
    }

#define MEGA	1000000
#define GIGA	1000000000

static char const *why_cull(TOD *t,TSI *i)
{
    int		age ;
    double	progress_min ;
    int64	done_min ;
    int		log ;

    if (i->progress > 90) return(0) ;

    age = tsg_g.time->tv_sec - i->time_added ;
    if (i->progress < .1) return(age < 600 ? 0 : "Not started after 600 seconds") ;

    log = ilog10(i->total_size) ;
    progress_min = 10 - log ;
    done_min = 10 * MEGA ;
    if (i->progress < progress_min) {
	return 0 ;
	}
    else if (i->total_done < done_min) {
	return 0 ;
	}
    else if (i->progress < 5) {
	if (i->ratio < .025) return "ratio too low" ;
	}
    else if (i->progress < 10) {
	if (i->ratio < .05) return "ratio too low" ;
	}
    else if (i->progress < 20) {
	if (i->ratio < .1) return "ratio too low" ;
	}
    else if (i->progress < 30) {
	if (i->ratio < .15) return "ratio too low" ;
	}
    else if (i->progress < 40) {
	if (i->ratio < .2) return "ratio too low" ;
	}
    else if (i->progress < 50) {
	if (i->ratio < .25) return "ratio too low" ;
	}
    else if (i->progress < 60) {
	if (i->ratio < .3) return "ratio too low" ;
	}
    else if (i->progress < 80) {
	if (i->ratio < .4) return "ratio too low" ;
	}
    else {
	}
    return 0 ;
    }

static void thcp_schedule_init_ttmf(TOD *t,u32 a)
{
    TSI		*i = t->tsi_c ;
    t->hcp.period = check_period(t,i) ;
    if (t->hcp.period != 0) {
	tod_log_printf(ls_generic,t,"initial check %d ...\n",t->hcp.period) ;
	log_printg(ls_generic,"%!tod!s initial check %d ...\n",t,t->hcp.period) ;
	t->hcp.state.enable = 1 ;
	}
    }

static void thcp_tod_disable(TOD *t)
{
    t->hcp.state.enable = 0 ;
    t->hcp.period = 0 ;
    log_printf(ls_generic,"%s hcp_tod_disable\n",t->hash) ;
    }

static void thcp_tod_init(TOD *t)
{
    t->hcp.state.enable = 1 ;
    t->hcp.period = 1 ;
    }

static void thcp_schedule_init(void)
{
    gettimeofday(tsg_g.time, NULL) ;
    tod_tab_map(thcp_schedule_init_ttmf,0) ;
    }

static void thcp_start()
{
    thcp_schedule_init() ;
    }

/* ================================================================ */
static void thmc_st_expire_map_fun(TOD *t,u32 a)
{
    TSG		*tsg = (TSG *) a ;
    TSI		*i = t->tsi_c ;
    char const	*why ;

    if (!t->hcp.state.enable) return ;

    if (t->hcp.period == 0) return ;	/* never check */
    if ((tsg_g.time->tv_sec % t->hcp.period) != 0) return ;

    if (why = why_cull(t,i)) {
	log_printf(ls_generic,"checking %s ... %6.2f %6.2f : ",t->hash,i->progress,i->ratio) ;
	log_printf_c(ls_generic,"culling (%s)\n",why) ;
	thmi_pause(t->hash) ;
	thcp_tod_disable(t) ;
	torrent_label(t,"--abort") ;
	}
    else {
	t->hcp.period = check_period(t,i) ;
	tsg->refresh = rcons_cons(t,tsg->refresh) ;
	}
//    log_printf_c(ls_generic,"next check %d\n",t->hcp.period) ;
    }


extern u32 thmc_st_expire(STIMER *st,u32 a)
{
    TSG		*tsg = (TSG *) st->c ;
    gettimeofday(tsg->time, NULL) ;
    tsg->refresh = 0 ;
    tod_tab_map(thmc_st_expire_map_fun,(u32) tsg) ;
{
    MT		hashlist[1] ;
    int		n = rcons_length(tsg->refresh) ;
    if (n > 0) {
	MTALLOCA(hashlist,n * 64) ;
	tod_rcons_qlistbuild(hashlist,tsg->refresh) ;
	dht_get_status_hashlist_mask(0,hashlist->s,-1) ;
	rcons_free_list(tsg->refresh) ;
	tsg->refresh = 0 ;
	}
}
    return(0) ;
}

static u32 thmc_wakeup_st(STIMER *st,int m,u32 a)
{
    switch(m) {
    case STM_EXPIRE:
	return(thmc_st_expire(st,a)) ;
	}
   return(0) ;
}

/* ================================================================ */
static void thcp_run_on(TSG *tsg)
{
    tsg->st = stimer_add(thmc_wakeup_st,tsg,1000) ;
    tsg->st->state |= STS_REARM ;
    tsg->running = 1 ;
    }

static void thcp_run_off(TSG *tsg)
{
    if (tsg->st) {
	stimer_kill(tsg->st) ;
	tsg->st = 0 ;
	}
    tsg->running = 0 ;
    }

static void thcp_run_toggle(TSG *tsg)
{
    if (tsg->running) 
	 thcp_run_off(tsg) ;
    else thcp_run_on(tsg) ;
    }

/* ================================================================ */
extern u32 thcp_state_vcf(void *c,int m,u32 a)
{
    VCFU *u = (VCFU *) &a ;
    TSG *tsg = &tsg_g ;
    switch(m) {
    case VCF_SETCHAR:
	return(thcp_state_vcf(c,switch_state_read(u->s),0)) ;
    case VCF_GETCHAR:
	switch_state_write(u->mt,tsg->running != 0) ;
	return 0 ;
    case VCF_SWITCH_ON:
	thcp_run_on(tsg) ;
	return(0) ;
    case VCF_SWITCH_OFF:
	thcp_run_off(tsg) ;
	return(0) ;
    case VCF_SWITCH_TOGGLE:
	thcp_run_toggle(tsg) ;
	return(0) ;
	}
    return(0) ;
    }

/* ================================================================ */
static void generic_json_response(UF *uf,HT *ht,RBP *r)
{
    json_t *ej = json_object_get(r->j, "error") ;
    json_t *rj = json_object_get(r->j, "result") ;
    const char *es = json_string_value(ej) ;
    const char *rs = json_string_value(rj) ;
    if (es) {
    }
    if (rs) {
    }
    }

/* ================================================================ */
LOGSEC torapi_ls[] = {"TORAPI",1} ;

static void torapi_accept_response(UF *uf,HT *ht,RBP *r)
{
    THMC	*c = (THMC *) ht->sc ;
    json_t *ej = json_object_get(r->j, "error") ;
    json_t *rj = json_object_get(r->j, "result") ;
    const char *es = json_string_value(ej) ;
    if (es) {
	errorshow((char *) es) ;
	return ;
	}
    (void) rj ;		/* should really parse response JSON for info */

    if (c->f)
	c->f(c,TSM_REPLY_COMPLETE,0) ;
    }

static u32 torapi_uff(UF *uf,int m,u32 a)
{
    HT		*ht = uf->d.ht ;
    THMC	*c = (THMC *) ht->sc ;
    switch(m) {
    case UFM_ARG_DESCRIBE: {
	UFM_ARG_DESCRIBE_ARG *aa = (typeof(aa)) a ;
	switch(aa->m) {
	case UFM_CREATE: {
	    THMC *cc = (typeof(cc)) aa->a ;
	    mtprintf(aa->mt,"%s ",cc->method) ;
	    return(0) ;
	    }
	    }
	break ;
	}
    case UFM_TYPENAME:	return(mtputs((MT *) a,"TORCMD")) ;
    case UFM_CREATE:
	ht->sc = (void *) (c = (THMC *) a) ;
	break ;
    case UFM_DESTROY:
	thmc_destroy_and_free(c) ;
	break ;
    case UFM_ACTION:
	if (!g.auth.cookie) return(0) ;
	break ;
    case UFM_HT_RESPONSE_JSON:
	torapi_accept_response(uf,ht,(RBP *) a) ;
	break ;
    default:
	break ;
	}
    return(dht_uff(uf,m,a)) ;
    }

/* ================================================================ */
/* thmi ---> Torrent Hash Method Invoke */
/* tlmi ---> Torrent List Method Invoke */

static void dht_tlmi(THMF f,char *method,char *hashlist,char *params,UFF uff)
{
    MT		mt[1] ;
    UF		*uf ;
    THMC	*c = tscalloc(1,THMC) ;
    MTALLOCA(mt,DHT_MAX) ;
    c->method = method ;
    c->f = f ;
    mtprintf(mt,"{\"method\":\"%s\"",c->method) ;
    mtputs(mt,",\"params\":") ;
    mtprintf(mt,"[[%s]",hashlist) ;
    if (!params) params = "" ;
    if (*params)
	mtprintf(mt,",%s",params) ;
    mtputs(mt,"],\"id\":%d}") ;
    log_printf(torapi_ls,"request: %s %s %s\n",hashlist,method,params) ;
    uf = deluge_ht_create(uff,mt->s,c) ;
    ufs_rc_add(uf) ;
    }
    
static void dht_thmi(THMF f,char *method,char *hash,char *params,UFF uff)
{
    MT		hashlist[1] ;
    MTALLOCA(hashlist,64) ;
    qlistbuild(hashlist,1,&hash) ;
    dht_tlmi(f,method,hashlist->s,params,uff) ;
    }
    
/* ================================================================ */
/* THOS ---> Torrent Hash Option Set */
/* TLOS ---> Torrent List Option Set */

static void dht_tlos(THMF f,char *hashlist,char *params,UFF uff)
{
    dht_tlmi(f,"core.set_torrent_options",hashlist,params,uff) ;
}

static void dht_thos(THMF f,char *hash,char *params,UFF uff)
{
    MT	hashlist[1] ;
    MTALLOCA(hashlist,DHT_MAX) ;
    qlistbuild(hashlist,1,&hash) ;
    dht_tlmi(f,"core.set_torrent_options",hashlist->s,params,uff) ;
}

/* ================================================================ */
extern void thmi_pause(char *hash)
{
    dht_thmi(0,"core.pause_torrent",hash,0,torapi_uff) ;
}
    
static void thmi_resume(char *hash)
{
    dht_thmi(0,"core.resume_torrent",hash,0,torapi_uff) ;
}
    
extern void thmi_remove(char *hash,char *params)
{
    dht_thmi(0,"core.remove_torrent",hash,params,torapi_uff) ;
}
    
/* ================================================================ */
/* ================================================================ */
/* ================================================================ */
static u32 throttle_done(THMC *c,int m,u32 a)
{
    switch (m) {
    case TSM_REPLY_COMPLETE: {
	return(0) ;
	}
    default:
	break ;
	}
    return(0) ;
} 

static void thmi_throttle(char *hash,int rate)
{
    MT	mt[1] ;
    MTALLOCA(mt,DHT_MAX) ;
    mtputs(mt,"{") ;
    buildargpair(mt,"max_download_speed","%d",rate) ;
    mtputs(mt,"}") ;
    dht_thos(throttle_done,hash,mt->s,torapi_uff) ;
    }
    
static void thmi_unthrottle(char *hash)
{
    thmi_throttle(hash,-1) ;
}
    
/* ================================================================ */
/* ================================================================ */
/* IPC ---> Initial Peer Count */

#define TC_IPC_THROTTLE		10

LOGSEC ls_torrent_ipc[] = {"TORIPC",1} ;

static u32 tc_ipc_tssf(UF *uf,int m,u32 a)
{
    switch (m) {
    case TSM_REPLY_COMPLETE: {
	TOD *tod = (TOD *) a ;
	TSI *tsi = tod->tsi_c ;
	tod_log_printf(ls_torrent_ipc,tod," peers = %d\n",tsi->num_peers) ;
	log_printg(ls_torrent_ipc,"%{tod}s peers = %d\n",tod,tsi->num_peers) ;
	if (tsi->num_peers < 2) {
	    thmi_throttle(tod->hash,TC_IPC_THROTTLE) ;
	    thmi_pause(tod->hash) ;
	    torrent_label(tod,"--boring") ;
	    thcp_tod_disable(tod) ;
	}
	else {
	    thmi_unthrottle(tod->hash) ;
	    thcp_tod_init(tod) ;
	    }
	return(0) ;
	}
    default:
	break ;
	}
    return(0) ;
}

static u32 tc_ipc_wakeup_st(STIMER *st,int m,u32 a)
{
    TOD *tod = (TOD *) st->c ;
    switch(m) {
    case STM_EXPIRE:
	feral_torrent_download(tod->hash) ;
	dht_get_status_hash_mask(tc_ipc_tssf,tod->hash,-1) ;
	return(0) ;
	}
    return(0) ;
    }
    
/* ================================================================ */
/* ================================================================ */
static void torrent_label_response(UF *uf,HT *ht,RBP *r)
{
    generic_json_response(uf,ht,r) ;
    }

static u32 torrent_label_uff(UF *uf,int m,u32 a)
{
    HT		*ht = uf->d.ht ;
    switch(m) {
    case UFM_TYPENAME:	return(mtputs((MT *) a,"LABEL")) ;
    case UFM_HT_RESPONSE_JSON:
	torrent_label_response(uf,ht,(RBP *) a) ;
	break ;
    default:
	break ;
	}
    return(dht_uff(uf,m,a)) ;
    }

extern void torrent_label(TOD *tod,char *label)
{
    MT		mt[1] ;
    UF		*uf ;
    char *t = "{\"method\":\"label.set_torrent\",\
\"params\":\
[\"%s\",\"%s\"\
],\
\"id\":%%d\
}\
" ;
    MTALLOCA(mt,DHT_MAX) ;
    mtprintf(mt,t,tod->hash,label) ;
    uf = tod_deluge_ht_create(torrent_label_uff,mt->s,tod) ;
    ufs_rc_add(uf) ;
    }

/* ================================================================ */
/* ================================================================ */
LOGSEC ls_torrent_add[] = {"ADD",1} ;

typedef struct {
    int		tid ;
    TOD		*tod ;
    } TADC ;

#define TC_IPC_TIMEOUT	8000

static void torrent_add_response(UF *uf,HT *ht,RBP *r)
{
    TADC	*tc = (TADC *) ht->sc ;
    json_t *ej = json_object_get(r->j, "error") ;
    json_t *rj = json_object_get(r->j, "result") ;
    const char *es = json_string_value(ej) ;
    const char *rs = json_string_value(rj) ;
    if (es) {
	tc->tod = 0 ;
	return ;
	}
    if (rs) {
	TOD	*tod ;
	tc->tod = tod = tod_intern((char *) rs) ;
	log_printf(ls_torrent_add,"torrent hash = %s\n",rs) ;
	tod->tid = tc->tid ;
	db_tsi_load_or_create(tod) ;
	uf_send(uf,UFM_TORRENT_ADDED,(u32) tod) ;
	}
    else {
	/* has torrent already been loaded ? */
	}
    }

static u32 legacy_torrent_added(UF *uf,TOD *tod)
{
    torrent_label(tod,"--new-1") ;
    tod->st = stimer_add(tc_ipc_wakeup_st,tod,TC_IPC_TIMEOUT) ;
    return 0 ;
    }

static u32 torrent_add_uff(UF *uf,int m,u32 a)
{
    HT		*ht = uf->d.ht ;
    TADC	*sc = (TADC *) ht->sc ;
    switch(m) {
    case UFM_DESCRIBE:
	if (sc)
	     return(mtprintf((MT *) a,"ADD(%d)",sc->tid)) ;
	else return(mtprintf((MT *) a,"ADD")) ;
    case UFM_TYPENAME:	return(mtputs((MT *) a,"ADD")) ;
    case UFM_CREATE:
	ht->sc = (void *) a ;
	break ;
    case UFM_DESTROY:
	free(ht->sc) ;
	break ;
    case UFM_ACTION:
	if (!g.auth.cookie) return(0) ;
	break ;
    case UFM_HT_RESPONSE_HDR:
	break ;
    case UFM_HT_RESPONSE_JSON:
	torrent_add_response(uf,ht,(RBP *) a) ;
	break ;
    case UFM_TORRENT_ADDED:
	if (uf_parent_notify(uf,UFM_POST_NOTIFY,m,a)) return(1) ;
	return(legacy_torrent_added(uf,(TOD *) a)) ;
    default:
	(void) sc ;
	break ;
	}
    return(dht_uff(uf,m,a)) ;
    }

extern UF *dht_add_torrent_url(char *id)
{
    MT		mt[1] ;
    TADC	*sc ;
    UF		*uf ;
    HT		*ht ;
    char *t = "{\"method\":\"core.add_torrent_url\",\
\"params\":\
[\"https://what.cd/torrents.php?action=download&id=%s&authkey=4811b35a4613a8d8e32924e371929154&torrent_pass=lh6umeb41zsccsi7xdizmedvjinubbia\",\
 {\
     \"add_paused\":%s,\
     \"max_download_speed\":-1\
 },\
 {}\
],\
\"id\":%%d\
}\
" ;
    MTALLOCA(mt,DHT_MAX) ;
    mtprintf(mt,t,id,tfstring(gopt.add_paused)) ;
    sc = tsmalloc(TADC) ;
    sc->tid = strtol(id,0,10) ;
    uf = deluge_ht_create(torrent_add_uff,mt->s,sc) ;
    ht = uf->d.ht ;
    log_printf(ls_torrent_add,"adding torrent %s seq: %d\n",id,ht->seq) ;
    ufs_rc_add(uf) ;
    return uf ;
    }

extern UF *dht_add_torrent_url_i(int id)
{
    MT mt[1] ;
    MTALLOCA(mt,16) ;
    mtprintf(mt,"%d",id) ;
    return dht_add_torrent_url(mt->s) ;
    }

/* ================================================================ */
/* ================================================================ */
#include	"sf.h"
#include	"sql.h"

typedef struct {
    SFT		sft ;
    void	*a ;
    int		(*bind)(sqlite3_stmt *,int,void *) ;
    } DBSF ;

/* ================================================================ */
static int dbsf_charp_bind(sqlite3_stmt *s,int i,void *o)
{
    return sqlite3_bind_cstring(s,i,*((char **) o)) ;
    }

DBSF dbsf_charp[] = {SFT_CHARP,0,dbsf_charp_bind} ;

/* ================================================================ */
static int dbsf_int_bind(sqlite3_stmt *s,int i,void *o)
{
    return sqlite3_bind_int(s,i,*((int *) o)) ;
    }

DBSF dbsf_int[] = {SFT_INT,0,dbsf_int_bind} ;

/* ================================================================ */
static int dbsf_int64_bind(sqlite3_stmt *s,int i,void *o)
{
    return sqlite3_bind_int64(s,i,*((int64 *) o)) ;
    }

DBSF dbsf_int64[] = {SFT_INT64,0,dbsf_int64_bind} ;

/* ================================================================ */
static int dbsf_bool_bind(sqlite3_stmt *s,int i,void *o)
{
    return sqlite3_bind_int(s,i,*((int *) o)) ;
    }

DBSF dbsf_bool[] = {SFT_BOOL,0,dbsf_bool_bind} ;

/* ================================================================ */
static int dbsf_double_bind(sqlite3_stmt *s,int i,void *o)
{
    return sqlite3_bind_double(s,i,*((double *) o)) ;
    }

DBSF dbsf_double[] = {SFT_DOUBLE,0,dbsf_double_bind} ;

/* ================================================================ */
static int dbsf_ENUM_TORRENTSTATE_bind(sqlite3_stmt *s,int i,void *o)
{
    return sqlite3_bind_int(s,i,*((int *) o)) ;
    }

DBSF dbsf_ENUM_TORRENTSTATE[] = {SFT_ENUM_TORRENTSTATE,0,dbsf_ENUM_TORRENTSTATE_bind} ;

/* ================================================================ */
extern DBSF *sft_dbsf(SFT sft)
{
    switch(sft) {
    case SFT_CHARP:	return(dbsf_charp) ;
    case SFT_INT:	return(dbsf_int) ;
    case SFT_INT64:	return(dbsf_int64) ;
    case SFT_BOOL:	return(dbsf_bool) ;
    case SFT_DOUBLE:	return(dbsf_double) ;
    case SFT_ENUM_TORRENTSTATE:	return(dbsf_ENUM_TORRENTSTATE) ;
    default:		errorfatal("unknown SFT %d\n",sft) ;
    }
    return(0) ;
    }
    
/* ================================================================ */
/* ================================================================ */
extern void tsi_add_mask_json(MT *mt,u32 mask)
{
    int		i,m = 1 ;
    mask &= ((1 << SFI_TSI__END) - 1) ;
    for (i = 0 ; i < SFI_TSI__END ; i++) {
	if (mask & m) {
	    mtputc(mt,'"') ;
	    mtputs(mt,TSI_sf[i].name) ;
	    mtputc(mt,'"') ;
	    if (mask &= ~m) mtputc(mt,',') ;
	    }
	m <<= 1 ;
	}
    mtputc(mt,0) ;
    }

extern int tsi_fname_to_index(char *name)
{
    int		i ;
    for (i = 0 ; i < SFI_TSI__END ; i++) {
	if (!strcmp(TSI_sf[i].name,name)) return i ;
	}
    return -1 ;
    }

extern u32 tsi_fname_list_to_mask(char *names)
{
    if (!names || !*names) return -1 ;
    names = strdup(names) ;
{
    char	*s = names ;
    char	*p = s ;
    char	*e ;
    int		i ;
    int		mask = 0 ;
    int		last = 0 ;
    while (!last) {
	s = p ;
	while (*p && *p != ',') p++ ;
	if (*p == 0) { last = 1 ;}
	while (s < p && *s == ' ') s++ ;
	e = p ;
	while (e > s && *e == ' ') e-- ;
	*e = 0 ;
	if ((i = tsi_fname_to_index(s)) != -1)
	    mask |= (1 << i) ;
	p++ ;
	}
    free(names) ;
    return mask ;
    }
    }

static void add_tsi_conversion(MT *mt,int i)
{
    switch(i) {
    case SFI_TSI_time_added:
	mtputs(mt,"datetime(?, 'unixepoch')") ;
	break ;
    default:
	mtputc(mt,'?') ;
	break ;
	}
    }

extern void tsi_add_mask_update(MT *mt,u32 mask)
{
    int		i,m = 1 ;
    mask &= ((1 << SFI_TSI__END) - 1) ;
    for (i = 0 ; i < SFI_TSI__END ; i++) {
	if (mask & m) {
	    mtprintf(mt,"%s=",TSI_sf[i].name) ;
	    add_tsi_conversion(mt,i) ;
	    if (mask &= ~m) mtputc(mt,',') ;
	    }
	m <<= 1 ;
	}
    mtputc(mt,0) ;
    }

/* ================================================================ */
/* example masks */
/* order doesn't matter here */

#define ALL_MASK \
    (1 << SFI_TSI_queue) |\
    (1 << SFI_TSI_name) |\
    (1 << SFI_TSI_total_size) |\
    (1 << SFI_TSI_state) |\
    (1 << SFI_TSI_progress) |\
    (1 << SFI_TSI_num_seeds) |\
    (1 << SFI_TSI_total_seeds) |\
    (1 << SFI_TSI_num_peers) |\
    (1 << SFI_TSI_total_peers) |\
    (1 << SFI_TSI_download_payload_rate) |\
    (1 << SFI_TSI_upload_payload_rate) |\
    (1 << SFI_TSI_eta) |\
    (1 << SFI_TSI_ratio) |\
    (1 << SFI_TSI_distributed_copies) |\
    (1 << SFI_TSI_is_auto_managed) |\
    (1 << SFI_TSI_time_added) |\
    (1 << SFI_TSI_tracker_host) |\
    (1 << SFI_TSI_save_path) |\
    (1 << SFI_TSI_total_done) |\
    (1 << SFI_TSI_total_uploaded) |\
    (1 << SFI_TSI_max_download_speed) |\
    (1 << SFI_TSI_max_upload_speed) |\
    (1 << SFI_TSI_seeds_peers_ratio) |\
    (1 << SFI_TSI_label) |\
    0

#define TMASK \
    (1 << SFI_TSI_time_added) |\
    0

/* ================================================================ */
/* ================================================================ */
typedef struct {
    TSMF	f ;
    TOD		*tod ;
    u32		mask ;
    sqlite3_stmt *stmt ;
    } DGTS_C ;

static void dgts_bind(DGTS_C *sc,TOD *tod)
{
    TSI *tsi = tod->tsi_c ;
    SF	*f ;
    int col = 1 ;
    int mask = 1 ;
    for (f = TSI_sf ; f->sft != SFT__END ; f++) {
	if (sc->mask & mask) {
	    DBSF *d = sft_dbsf(f->sft) ;
	    d->bind(sc->stmt,col++,SF_P(f,tsi)) ;
	}
	mask <<= 1;
    }
    sqlite3_bind_cstring(sc->stmt,col,tod->hash) ;
}

static void dgts_sql_step(DGTS_C *sc,TOD *tod)
{
    int		r ;
    db_tsi_load_or_create(tod) ;
    dgts_bind(sc,tod) ;
    r = sqlite3_step_or(OR_WARN,sqlgp()->dbh,sc->stmt) ;
    r = sqlite3_reset(sc->stmt) ;
}

static u32 dgts_uff(UF *uf,int m,u32 a)
{
    HT		*ht = uf->d.ht ;
    DGTS_C	*sc = (DGTS_C *) ht->sc ;
    switch(m) {
    case UFM_TYPENAME:	return(mtputs((MT *) a,"DGTS")) ;
    case UFM_CREATE:
	ht->sc = (void *) a ;
	break ;
    case UFM_DESTROY:
	sqlite3_finalize(sc->stmt) ;
	transaction_end() ;
	free(sc) ;
	break ;
    case UFM_ACTION:
	break ;
    case UFM_HT_RESPONSE_JSON:
	transaction_begin() ;
	tlsi_accept_response(uf,ht,(RBP *) a) ;
	return(0) ;
    case UFM_HT_TORRENT_STATUS_GET:
	dgts_sql_step(sc,(TOD *) a) ;
	sc->f(uf,TSM_REPLY_COMPLETE,a) ;
	return(0) ;
    default:
	break ;
	}
    return(dht_uff(uf,m,a)) ;
    }

extern u32 null_tsmf(UF *uf,int m,u32 a)
{
    return 0 ;
    }

extern UF *dht_get_status_filter_mask(TSMF f,char *filter,int mask)
{
    MT		mt[1] ;
    MT		mta[1] ;
    UF		*uf ;
    DGTS_C	*sc = tscalloc(1,DGTS_C) ;
    char *t = "{\"method\":\"core.get_torrents_status\",\
\"params\":[\
{%s},\
[%s]\
],\
\"id\":%%d\
}\
" ;
    if (!f) f = null_tsmf ;
    sc->f = f ;
    sc->mask = mask ;

    MTALLOCA(mta,1024) ;
    tsi_add_mask_json(mta,sc->mask) ;

    MTALLOCA(mt,1024) ;
    mtprintf(mt,t,filter,mta->s) ;
    uf = deluge_ht_create(dgts_uff,mt->s,(void *) sc) ;

    MTREWIND(mta) ;
    tsi_add_mask_update(mta,sc->mask) ;
    MTREWIND(mt) ;
    mtprintf(mt,"update tstatus set %s where hash = ?",mta->s) ;

    sqlite3_prepare_v2_or(OR_WARN,sqlgp()->dbh,mt->s,-1,&sc->stmt,0) ;

    ufs_rc_add(uf) ;
    return(uf) ;
    }

extern UF *dht_get_status_everything(TSMF f)
{
    return dht_get_status_filter_mask(f,"",ALL_MASK) ;
    }

extern UF *dht_get_status_hash_mask(TSMF f,char *hash,int mask)
{
    MT		filter[1] ;
    MTALLOCA(filter,256) ;
    param_add(filter,"id",hash) ;
    return(dht_get_status_filter_mask(f,filter->s,mask)) ;
    }

extern UF *dht_get_status_hashlist_mask(TSMF f,char *hashlist,int mask)
{
    MT		filter[1] ;
    MTALLOCA(filter,256) ;
    param_add_u(filter,"id",hashlist) ;
    return(dht_get_status_filter_mask(f,filter->s,mask)) ;
    }

/* ================================================================ */
/* ================================================================ */
static u32 uff_dbcreate(UF *uf,int m,u32 a)
{
    typedef struct {
	int	phase ;
	} UFC ;
    UFC *c = (UFC *) uf->d.v ;

    switch(m) {
    case UFM_TYPENAME:	return(mtputs((MT *) a,"DBCREATE")) ;
    case UFM_CREATE:
	c = tscalloc(1,UFC) ;
	uf->d.v = (void *) c ;
	c->phase = 0 ;
	return(0) ;
    case UFM_DESTROY:
	free(c) ;
	break ;
    case UFM_POST_NOTIFY: {
	UFM_NOTIFY_ARG *na = (UFM_NOTIFY_ARG *) a ;
	switch(na->m) {
	case UFM_DESTROY:
	    switch (++c->phase) {
	    /* note uf_destroy only on last phase */
	    case 1:
		thcp_start() ;
		break ;
	    default:
		break ;
	    }
	    uf_destroy(uf) ;
	}
	return(0) ;
	}
    default:
	break ;
	}
    return(null_uff(uf,m,a)) ;
    }

extern void tod_tab_populate_dht(void)
{
    UF *ufp ;
    UF *uf ;
    ufp = uf_create(uff_dbcreate,0,0) ;
    uf = dht_get_status_everything(0) ;
    uf->parent = ufp ;
}

/* ================================================================ */
static void ua_init(void)
{
    g.http_status_pat.s = "HTTP/1.1 \\(\\d+\\) " ;
    g.http_status_pat.r->regexp = 1 ;
    re_compile_pattern_s(g.http_status_pat.r,g.http_status_pat.s) ;
    }

static void ua_finish(void)
{
    re_free(g.http_status_pat.r) ;
    }

/* ================================================================ */
#include	<pu/printg.h>

static int pff_tod(char *pb,int cb,FMTSPEC *spec,TOD *tod)
{
    MT		mt[1] ;
    MTSETN(mt,pb,cb) ;
    return tod_identify(mt,tod) ;
    }

extern void dht_init(void)
{
    printg_register("tod",(PFF_S) pff_tod) ;
    ua_init() ;
    dht_auth() ;
    tod_tab_init() ;
    tod_tab_populate_db() ;
    tod_tab_populate_dht() ;
    dht_session_plugin() ;
}

extern void dht_onexit(void)
{
    dht_session_unplug() ;
    tod_tab_destroy() ;
    ua_finish() ;
    freenz(g.auth.cookie) ;
    }



#ifdef __cplusplus /*Z*/
}
#endif
