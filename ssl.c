#ifdef __cplusplus /*Z*/
extern "C" {
#endif

#include	<openssl/ssl.h>
#include	<openssl/bio.h>
#include	<openssl/err.h>

#include	<pu/mt.h>

#include	"arg.h"
#include	"errors.h"

#include	"common.h"

/* ================================================================ */
typedef struct {
    BIO 	*conn ;
    int		state ;
    } SLC ;

#define SLCS_CREATE_FAIL	0x10000
#define SLCS_CONNECT_FAIL	0x20000

typedef struct {
    char	*smode ;
    int		state ;
    } SSLG ;

SSLG g ;

#define		PORT	"9001"
#define		SERVER	"localhost"

static void init_OpenSSL(void)
{
    SSL_library_init() ;	/* no error code */
}

extern char *mtfget(MT *mt,FILE *f)
{
    char	*s = fgets(mt->c,mt->e-mt->c,f) ;
    mt->c += strlen(s) ;
    return(s) ;
    }

/* ================================================================ */
static void client_loop(SLC *c)
{
    MT		mt[1] ;
    int n;
    char	*p ;
    MTALLOCA(mt,256) ;

    for (;;) {
	MTREWIND(mt) ;
	if (!mtfget(mt,stdin)) break;
	for (p = mt->s ; p < mt->c ; ) {
	    n = BIO_write(c->conn, p, mt->c - p);
	    if (n < 0) return;
	    if (n == 0) break;
	    p += n ;
	}
    }
}

static void connect(SLC *c,char *server,char *port)
{
    MT		mt[1] ;
    MTALLOCA(mt,1024) ;
    mtprintf(mt,"%s:%s",server,port) ;
    if (!(c->conn = BIO_new_connect(mt->s))) {
	c->state |= SLCS_CREATE_FAIL ;
	return ;
	}
    if (BIO_do_connect(c->conn) <= 0) {
	c->state |= SLCS_CONNECT_FAIL ;
	return ;
	}
    }

extern int sm_client(int argc,char **argv)
{
    SLC		c[1] ;
    init_OpenSSL() ;
    connect(c,SERVER,PORT) ;
    client_loop(c) ;
    return(0) ;
}

/* ================================================================ */
#include	"thread.h"

static void do_server_loop(BIO *conn)
{
    int n ;
    char buf[80];
    char *c,*s,*e ;
    e = buf+sizeof(buf) ;
    s = buf ;
    while (1) {
	for (c = s; c < e; c += n) {
	    n = BIO_read(conn, c, e-c);
	    fwrite(c, 1, n, stdout);
	    if (n <= 0) break;
	}
	fflush(stdout) ;
	if (n <= 0) break ;
    }
}

static void THREAD_CC server_thread(void *arg)
{
    BIO *client = (BIO *)arg;

    fprintf(stderr, "Connection opened.\n");
    do_server_loop(client);
    fprintf(stderr, "Connection closed.\n");

    BIO_free(client);

    }

static int sm_server(int argc, char *argv[])
{
    BIO *acc, *client;
    THREAD_TYPE tid;
    
    init_OpenSSL();
	
    acc = BIO_new_accept(PORT);
    if (!acc)
	errorshow("Error creating server socket");

    if (BIO_do_accept(acc) <= 0)
	errorshow("Error binding server socket");

    for (;;) {
	if (BIO_do_accept(acc) <= 0)
	    errorshow("Error accepting connection");

	client = BIO_pop(acc);
	THREAD_CREATE(tid, (void *(*)(void *)) server_thread, (void *) client);
    }

    BIO_free(acc);
    return 0;
    }

/* ================================================================ */
static int long_arg_fun(char *name,char *value,void *a0)
{
    return(ASF_ARGIGNORED) ;
    }

static int ssh_main(int argc,char **argv,char *mode)
{
    int		i ;
    for (i = 1 ; i < argc ; i++) {
	char *arg = argv[i] ;
	if (!arg) continue ;		/* previously killed */
	if (*arg == '-') {
	    switch (*(arg+1)) {
	    case '-':
		arg_long(argc-i,argv+i,long_arg_fun,0) ;
		break ;
	    case 's':
		g.smode = (arg+2) ;
		argv[i] = 0 ; 
		break ;
	    default:
		break ;
	    }
	    }
	}
    if (!g.smode || !strcmp(g.smode,"c")) return(sm_client(argc,argv)) ;
    if (!strcmp(g.smode,"s")) return(sm_server(argc,argv)) ;
    errorshow("unknown submode %s\n",g.smode) ;
    return(RC_OK) ;
    }

/* ~# use mainmode ; #~ */
/* ~~mode("ssh",
   desc		=> "ssl client test",
   )~~ */

#include	".gen/ssl.c"

#ifdef __cplusplus /*Z*/
}
#endif
