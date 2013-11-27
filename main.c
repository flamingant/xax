#ifdef __cplusplus
extern "C" {
#endif

#include	<unistd.h>
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>

#include	"arg.h"
#include	"dp.h"

#include	"common.h"
#include	"atinit.h"

static struct {
    int		version_show ;
    char	*mode ;
    int		help ;
    } g = {
    .version_show = 0,
    } ;

#include	"version.h"

static void version_show(void)
{
    printf("version %d.%d build %d\n",version.major,version.minor,version.build) ;
    }

/* ================================================================ */
/* ~# use arg ; #~ */

/* ~~argset(help => "Main")~~ */

/* ~~arg(argname => 'm',help => "Set main mode of program",value => "MODE",inline => 1,short => 1)~~ */
/* ~~arg(help => "Set main mode of program",value => "MODE")~~ */
static int argf__mode(char *name,char *value,void *a0)
{
    g.mode = value ;
    return(ASF_ARGACCEPTED) ;
}

/* ~~arg(help => "Display this help")~~ */
static int argf__help(char *name,char *value,void *a0)
{
    g.help = 1 ;
    return(ASF_DEFERRED) ;		/* keep arg for mode help */
}

#include	".gen/main.h"

/* ================================================================ */
static int long_arg_fun(char *name,char *value,void *a0)
{
    if (!strcmp(name,"test-echo")) {
	printf("test-echo %s\n",value) ;
	return(ASF_ARGACCEPTED) ;
	}
    return(argitem_try(args_main,name,value,a0)) ;
    }

/* ~~arg(argname => 'h',help => "Display this help",inline => 1,short => 1)~~ */
/* ~~arg(argname => 'd',help => "Change directory to DIR",value => "DIR",inline => 1,short => 1)~~ */
/* ~~arg(argname => 'v',help => "Show version and exit",inline => 1,short => 1)~~ */

static int main_0(int argc,char **argv)
{
    int		i ;
    argc = argset_try_all(argc,argv) ;

    if (g.version_show) version_show() ;
    for (i = 1 ; i < argc ; i++) {
	char *arg = argv[i] ;
	if (!arg) continue ;		/* previously killed */
	if (*arg == '-') {
	    switch (*(arg+1)) {
	    case '-':
		arg_long(argc-i,argv+i,long_arg_fun,0) ;
		break ;
	    case 'h':
		g.help = 1 ;
		/* keep arg for mode help */
		break ;
	    case 'd':
		if (chdir(arg+2) == -1) {
		    errorfatal("failed to chdir to %s\n",arg+2) ;
		    }
		argv[i] = 0 ;
		break ;
	    case 'm':
		g.mode = (arg+2) ;
		argv[i] = 0 ; 
		break ;
	    case 'v':
		if (!g.version_show) version_show() ;
		exit(0) ;
		break ;
	    default:
		break ;
	    }
	    }
	}
    return(RC_OK) ;
    }

/* ================================================================ */
#include	"mainmode.h"

static void mmo_map(void (*fun)(MMO *,u32),u32 a)
{
    extern MMO *mainmode_initvec[] ;
    MMO **mm = mainmode_initvec ;
    MMO *m ;
    while (m = *mm) {fun(m,a) ; mm++ ;}
    }

static MMO *modemain_find(char *mode)
{
    extern MMO *mainmode_initvec[] ;
    MMO **mm = mainmode_initvec ;
    MMO *m ;
    while ((m = *mm) && (!m->enabled || !(m->f(m,MMM_MATCH_NAME,(u32) mode)))) mm++ ;
    return(m) ;
    }

/* ================================================================ */
static void help_common()
{
    printf("tc - torrent controller\n\n") ;
    }

static void help_mode(char *mode)
{
    }

static void help_show_modes_(MMO *m,u32 a)
{
    if (m->enabled)
	printf("   -m%-16s %s\n",mmo_getname(m),mmo_getdesc(m)) ;
}

static void help_show_modes()
{
    printf("modes:\n") ;
    mmo_map(help_show_modes_,0) ;
    }

static void help_argset(MT *mto,ARGSET *as)
{
    as->omf(as,OM_GETHELP,(u32) mto) ;
    }

static void help_mode_all()
{
    extern ARGSET *argset_initvec[] ;
    ARGSET **as ;
    MT mt[1] ;
    mtmalloc(mt,65536) ;

    help_show_modes() ;
    printf("\n") ;

    for (as = argset_initvec ; *as ; as++) {
	MTREWIND(mt) ;
	help_argset(mt,*as) ;
	printf("%s",mt->s) ;
	}

    printf("\n") ;
    mtfree(mt) ;
    }

/* ================================================================ */
#include	<pu/filename.h>

static int main_final(int argc,char **argv)
{
    MMO *mm ;
    if (g.help) {
	help_common() ;
	if (g.mode)
	     help_mode(g.mode) ;
	else help_mode_all() ;
	return(0) ;
	}
    if (!g.mode) {
	char *tmp = filename_nopath(*argv) ;
	char *mode = (char *) alloca(strlen(tmp)) ;
	filename_prefix_copy(mode,tmp) ;
	g.mode = (char *) alloca(strlen(mode)+1) ;
	strcpy(g.mode,mode) ;
	if (!(mm = modemain_find(mode)) &&
	    !(mm = modemain_find(0)))
	    errorfatal("no default mode\n") ;
	}
    else {
	mm = modemain_find(g.mode) ;
	if (!mm) errorfatal("unknown mode '%s'\n",g.mode) ;
    }

    argc = arg_compress(argc,argv) ;
{
    MMENTRY entry = (MMENTRY) mm->f(mm,MMM_GET_ENTRY,0) ;
    return(entry(argc,argv,g.mode)) ;
    }
    }

/* ================================================================ */
extern int main(int argc, char**argv)
{
    int r ;
    arg_expand(&argc,&argv) ;
    if ((r = main_0(argc,argv)) != RC_OK) return(r) ;
    if ((r = main_final(argc,argv)) != RC_OK) return(r) ;
    return(r) ;
}

/* ~~argset(name => "debug",help => "Debug output settings")~~ */
/* ~~arg(help => "Set debug output file",value => "FILE")~~ */
static int argf__debug_output(char *name,char *value,void *a0)
{
    dpf_file_set(value) ;
    return(1) ;
}

/* ~~arg(help => "Set debug verbosity level",value => "LEVEL")~~ */
static int argf__debug_level(char *name,char *value,void *a0)
{
    gdbg.level = arg_read_int_or_die(name,value) ;
    return(1) ;
}

/* ~~arg(help => "Set debug enable mask",value => "MASK")~~ */
static int argf__debug_mask(char *name,char *value,void *a0)
{
    gdbg.mask = arg_read_int_or_die(name,value) ;
    return(1) ;
    }

/* ================================================================ */
extern int mmo_help_generic(MMO *o,MT *mt)
{
    return 0 ;
    }

extern u32 mmf_null(MMO *o,int m,u32 a)
{
    switch(m) {
    case MMM_GET_ENTRY:	
	return((u32) o->entry) ;
    case MMM_GET_NAME:	
	return((u32) o->name) ;
    case MMM_GET_DESC:	
	return((u32) o->desc) ;
    case MMM_MATCH_NAME:
	if (a == 0) return(o->default_mmo) ;
	return(!strcmp((char *) a,(char *) o->f(o,MMM_GET_NAME,a))) ;
    case MMM_GET_HELP:
	return(mmo_help_generic(o,(MT *) a)) ;
	}
    return(0) ;
    }
    
#include	".gen/main.c"

#ifdef __cplusplus
}
#endif

