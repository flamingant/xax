#ifdef __cplusplus /*Z*/
extern "C" {
#endif

#include	<string.h>
#include	<unistd.h>
#include	<stdio.h>
#include	<stdlib.h>

#include	"arg.h"
#include	"common.h"

#include	".gen/mg.h"
/* ================================================================ */
static struct {
    char	*smode ;
    int		cpid ;
    int		gpid ;
    struct {
	int	dummy ;
	} cf ;
    } g ;

/* ================================================================ */
/* ~# use arg ; #~ */
/* ~~argset(help => "Debug shell settings")~~ */

/* ~~arg(help => "Submode")~~ */
static int argf__submode(char *name,char *value,void *a0)
{
    g.smode = strdup(value) ;
    return(ASF_ARGACCEPTED) ;
}

/* ================================================================ */
static int gfork(int argc,char **argv)
{
    char	*path = "d:/g/gdb-python-7.5-1/bin/gdb-python27.exe" ;
    g.gpid = execl(path,
		   path,
		   "-i=mi",
		   "tc.exe",
		   (char *) 0) ;
    printf("%d shouldn't get here\n",getpid()) ;
    exit(1) ;
    return(RC_OK) ;
}

/* ================================================================ */
static int sm_a(int argc,char **argv)
{
    if (g.cpid = fork()) return gfork(argc,argv) ;
    printf("%d still in main\n",getpid()) ;
    return(RC_OK) ;
}

/* ================================================================ */
static int sm_b(int argc,char **argv)
{
    return(RC_OK) ;
}

/* ================================================================ */
#include	"mainmode.h"

static int arg_read(int argc,char **argv)
{
    return argset_try_one(argc,argv,argset_mg) ;
}

static int g_main(int argc,char **argv,MMC *c)
{
    int		i ;

    if (!c->noinit) {
	arg_expand_try_file(&argc,&argv,".mg") ;
	}

    for (i = 1 ; i < argc ; i++) {
	char *arg = argv[i] ;
	if (!arg) continue ;		/* previously killed */
	if (*arg == '-') {
	    switch (*(arg+1)) {
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
/* ~~mode("g",
   desc		=> "Debug shell mode",
)~~ */

#include	".gen/mg.c"

#ifdef __cplusplus /*Z*/
}
#endif
