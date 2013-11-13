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

#include	<fcntl.h>
#include	<errno.h>

#include	<sys/select.h>
#include	<sys/param.h>

#include	<stdarg.h>
#include	"pu/mt.h"
#include	"pu/rcons.h"
#include	"pu/regex.h"

#include	"pu/exithook.h"

#include	<sys/time.h>

#include	"log.h"

#include	"ma.h"
#include	"dht.h"

#include	"stimer.h"

#include	"jsf.h"
#include	"uf.h"
#include	"mag.h"

#include	"tc_sql.h"
#include	"ht_tc.h"

#define UF_ARG_WANT__UFM_ARG_DESCRIBE	1

#include	"uf_arg.h"

#include	"common.h"

/* ================================================================ */
#include	"tweak.h"

static struct {
    MAGPUB	pub[1] ;
    char	*smode ;
    struct {
	int	dummy ;
	} cf ;
    } g ;

extern MAGPUB *magpub(void)
{
    return g.pub ;
    }

/* ================================================================ */
static int sma_long_arg_fun(char *name,char *value,void *a0)
{
    return(ASF_ARGIGNORED) ;
    }

/* ================================================================ */
/* be aware this is called last of all */

static void sm_a_exit(void *a)
{
    stimer_onexit() ;
    rcons_leak_check() ;
}

static int sm_a(int argc,char **argv)
{
    exithook_install(sm_a_exit,0) ;
    log_init(argc,argv) ;
    sql_init() ;
    argc = ufs_init(argc,argv) ;
    argc = arg_read_simple(argc,argv,sma_long_arg_fun,0) ;
    
    hsg_init(argc,argv) ;
    dht_init() ;
    ufs_loop() ;
    ufs_destroy() ;
    dht_onexit() ;

    return(RC_OK) ;
}

/* ================================================================ */
static int sm_b(int argc,char **argv)
{
    return(RC_OK) ;
}

/* ================================================================ */
static int long_arg_fun(char *name,char *value,void *a0)
{
    return(ASF_ARGIGNORED) ;
    }

/* ================================================================ */
static int a_main(int argc,char **argv,char *mode)
{
    int		i ;

    if ((i = arg_peek(argc,argv,"--no-init")) == -1) {
	arg_expand_try_file(&argc,&argv,".ma") ;
	}
    else argv[i] = 0 ;

    for (i = 1 ; i < argc ; i++) {
	char *arg = argv[i] ;
	if (!arg) continue ;		/* previously killed */
	if (*arg == '-') {
	    switch (*(arg+1)) {
	    case '-':
		arg_long(argc-i,argv+i,long_arg_fun,0) ;
		break ;
	    case 'x':
		argv[i] = 0 ; 
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
    if (!g.smode || !strcmp(g.smode,"a")) return(sm_a(argc,argv)) ;
    if (!strcmp(g.smode,"b")) return(sm_b(argc,argv)) ;
    errorshow("unknown submode %s\n",g.smode) ;
    return(RC_OK) ;
    }

/* ================================================================ */
/* ~# use mainmode ; #~ */
/* ~~mode("a",
   desc		=> "Normal torrent monitoring mode (default)",
   default_mmo	=> 1,
)~~ */

#include	".gen/ma.c"

#ifdef __cplusplus /*Z*/
}
#endif
