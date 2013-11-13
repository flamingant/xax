#ifdef __cplusplus /*Z*/
extern "C" {
#endif

/*#= feature: */
/* H=============================================================== */
#include	<stdlib.h>
#include	<stdio.h>
#include	<string.h>
#include	<malloc.h>

#include	<pu/dir.h>
#include	<pu/number.h>
#include	<pu/filename.h>

#include	<pu/alloct.h>
#include	"dp.h"
#include	<pu/aa.h>

#include	<windows.h>

#include	<pu/mt.h>
#include	<pu/strtx.h>
#include	<pu/hash.h>

static struct {
    char	*apache_conf ;
    char	*emacs_exe ;
    } conf = {
    "d:/a/apache/conf/httpd.conf",
    "d:/e/emacs-22.1/emacs/bin/emacsclientw.exe",
    } ;

typedef struct {
    char	*tag ;
    int		(*fun)(MT *,MT *,char *) ;
    } EXETAB ;

typedef struct {
  char *old ;
  char *host ;
  char *drive ;
  char *path ;
  } US ;

US us[128] = {
    {0},
    } ;
int nus ;

/* ================================================================ */
static void url_to_file(MT *f)
{
    US *u ;
    MT o[1] ;
    MTALLOCA(o,MTGO(f)) ;
    mtput(o,f->s,MTGO(f)) ;
    for (u = us ; u->old ; u++) {
	char *p1 = u->old ;
	char *p2 = o->s ;
	while (1) {
	    if (!*p1) {
		MTREWIND(f) ;
		mtprintf(f,"//%s/%s/%s",u->host,u->drive,u->path) ;
		mtputs(f,p2) ;
		return ;
		}
	    if (tx_ucase[(uchar) *p1] != tx_ucase[(uchar) *p2]) break ;
	    p1++ ;
	    p2++ ;
	    }
        }
    }

static void unescape(MT *d,char *s)
{
    int state = 0,v ;
    char *a ;
    if (!(a = s)) return ;
    while (*a) {
       if (*a == '%') {
	  state = 1 ;
	  v = 0 ;
	  }
       else switch(state) {
       case 0: mtputc(d,*a) ; break ;
       case 1: v |= digit(*a) << 4 ; state++ ; break ;
       case 2: v |= digit(*a) ; mtputc(d,v) ; state = 0 ; break ;
       }
    a++ ;
    }
    mtputc(d,0) ;
    }

/* ================================================================ */
extern void filename_truncate_to_path(char *name)
{
    char *s = filename_last_path(name) ;
    if (s) *s = 0 ;
    }

/* ================================================================ */
extern STARTUPINFO *STARTUPINFO_default(STARTUPINFO *info)
{
    memset(info,0,sizeof(STARTUPINFO)) ;
    info->cb = sizeof(STARTUPINFO) ;
    return(info) ;
    }

static int invoke_CreateProcess(char *exe,char *cmd)
{
    int		r = 0 ;
    int flags = 0 ;
    int pid ;
    PROCESS_INFORMATION pinfo[1] ;
    STARTUPINFO sinfo[1] ;
    STARTUPINFO_default(sinfo) ;
    dpfn(1,"exe -> %s\n",exe) ;
    dpfn(1,"cmd -> %s\n",cmd) ;
    pid = CreateProcess(exe,cmd,
		       (SECURITY_ATTRIBUTES *) 0,(SECURITY_ATTRIBUTES *) 0,
		       TRUE,
		       flags,
		       0,
		       0,
		       sinfo,
		       pinfo) ;
    return(r) ;
    }

static HINSTANCE invoke_ShellExecute(char *verb,char *ufile,char *uargs)
{
    SHELLEXECUTEINFO i[1] ;
    MT file[1] ;
    MT dir[1] ;
    MT args[1] ;

    memset(i,0,sizeof(i)) ;

    i->cbSize = sizeof(SHELLEXECUTEINFO) ;
    i->fMask = 0 ;

    i->lpVerb = verb ;

    MTALLOCA(file,1024) ;
    unescape(file,ufile) ;
    filename_slashify(file->s,'\\') ;
    i->lpFile = filename_nopath(file->s) ;

    MTALLOCA(dir,1024) ;
    mtputs(dir,file->s) ;
    filename_truncate_to_path(dir->s) ;
    i->lpDirectory = dir->s ;

    MTALLOCA(args,1024) ;
    unescape(args,uargs) ;
    i->lpParameters = args->s ;

    i->nShow = SW_SHOW ;
{
    BOOL ee = ShellExecuteEx(i) ;
    if (ee) 
	return i->hInstApp ;
    else return 0 ;
}

/*
{
    HINSTANCE e ;
    e = ShellExecute(NULL,i->lpVerb,i->lpFile,i->lpParameters,i->lpDirectory,i->nShow) ;
    return(e) ;
    }
*/
    }

/* ================================================================ */
static void apache_conf_read(void)
{
    FILE *f = fopen(conf.apache_conf,"rt") ;
    fclose(f) ;
    }

typedef struct {
  char	*tag ;
  int	argc ;
  char	**argv ;
  } ARG ;

static void parse(ARG *a,MT *o,char *s)
{
  char *e,*lnb ;
  int argcmax ;
  mtputs(o,s) ;
  s = e = o->s ;	/* move start pointer to copy */
  
  a->argc = 0 ;
  a->tag = 0 ;
  a->argv = (char **) o->c ;

  argcmax = (o->e - o->c) / sizeof(char *) ;

  if (!(e = strchr(s,'='))) {return ;}
  *e++ = 0 ;
  a->tag = s ;
  while (1) {
    if (!*e) break ;
    if (a->argc >= argcmax) break ;

    while (*e == ' ' || *e == '\t') e++ ;
    if (!*e) break ;
    if (*e == '\n' || *e == '\r') break ;

    if (*e == '"' || *e == '\'') {
      char t = *e ;
      s = ++e ;
      while (*e && *e != t) e++ ;
      if (!*e) break ;
      a->argv[a->argc++] = s ;
      *e = 0 ;
      e++ ;
      }
    else {
      s = e++ ;
      lnb = e ;
      while (*e && *e != '\n' && *e != '\r') {
	if (*e == '\'') {if (!*++e) break ;}
	if (*e != ' ' && *e != '\t') lnb = e ;
	e++ ;
	}
      a->argv[a->argc++] = s ;
      *++lnb = 0 ;
    }
  }
}
	
static void conf_alias(ARG *a)
{
  US *u ;
  if (nus == 128) return ;
  if (a->argc < 4) return ;
  u = us+nus ;
  u->old = strdup(a->argv[0]) ;
  u->host = strdup(a->argv[1]) ;
  u->drive = strdup(a->argv[2]) ;
  u->path = strdup(a->argv[3]) ;
  nus++ ;
  }

static void conf_emacs_exe(ARG *a)
{
  if (a->argc < 1) return ;
  conf.emacs_exe = a->argv[0] ;
  }

static void conf_apache_conf(ARG *a)
{
  if (a->argc < 1) return ;
  conf.apache_conf = a->argv[0] ;
  apache_conf_read() ;
  }

static void conf_read(FILE *f)
{
    char s[1024] ;
    while (!feof(f)) {
      ARG a[1] ;
      MT o[1] ;
      mtmalloc(o,1024) ;
      fgets(s,sizeof(s),f) ;
      MTREWIND(o) ;
      parse(a,o,s) ;
      if (!a->tag) {
	continue ;
	}
      else if (!stricmp(a->tag,"alias")) {
	conf_alias(a) ;
	}
      else if (!stricmp(a->tag,"emacs_exe")) {
	conf_emacs_exe(a) ;
      }
      else if (!stricmp(a->tag,"apache_conf")) {
	conf_apache_conf(a) ;
      }
    }
}

static int conf_try(char *path)
{
    FILE *f ;
    MT o[1] ;
    MTALLOCA(o,64) ;
    mtprintf(o,"%s/oprun.conf",path) ;
    
    f = fopen(o->s,"rt") ;
    if (!f) return(0) ;
    dpfn(1,"using conf file %s\n",o->s) ;
    conf_read(f) ;
    fclose(f) ;
    return(1) ;
    }

static int conf_try_cwd(char *modname)
{
    MT o[1] ;
    MTALLOCA(o,64) ;
    mtprintf(o,modname) ;
    filename_strip_to_path(o->s) ;
    return(conf_try(o->s)) ;
    }

static void conf_try_all(char *modname)
{
    static int done ;
    if (done) return ;
    done = 1 ;
    if (conf_try_cwd(modname)) return ;
    if (conf_try("d:/bin")) return ;
    if (conf_try("e:/bin")) return ;
    }

#define SCHEME_HTTP	0
#define SCHEME_FILE	1

/* ================================================================ */
#include	"http.h"

/* ~# use http ; #~ */

/* ================================================================ */
typedef struct {
    char	*file ;
    char	*line ;
    char	*search ;
    char	*expr ;
    } EMARG ;

static int inserver_e(EMARG *e)
{
    char	*exe ;
    MT		cmd[1] ;
    MTALLOCA(cmd,1024) ;
    exe = conf.emacs_exe ;
    mtputs(cmd,"emacsclientw -n -e") ;
    mtprintf(cmd," \"(find-file \\\"%s\\\")\"",e->file) ;
    if (e->line) {
	mtprintf(cmd," \"(goto-line %s)\"",e->line) ;
    }
    if (e->search) {
	mtprintf(cmd," (bob) \"(rsf \\\"%s\\\")\"",e->search) ;
    }	
    return(invoke_CreateProcess(exe,cmd->s)) ;
}

/* ================================================================ */
static char *url_arg_to_file(MT *file,char *arg)
{
    if (!memcmp_icase(arg,"http:",5)) {
	arg += 5 ;
	unescape(file,arg) ;
	url_to_file(file) ;
	return(file->s) ;
    }
    else if (!memcmp_icase(arg,"file:",5)) {
	arg += 5 ;
	unescape(file,arg) ;
	return(file->s) ;
    }
    else {
	return arg ;
    }
    }

/* ~~hpf(help => "Invoke emacs edit on file")~~ */
static u32 hpf_emacs(HSR *r,int m,u32 a)
{
    switch (m) {
    case HPM_INTERNAL: {
	MT file[1] ;
	MTALLOCA(file,1024) ;
	EMARG e[1] ;
	e->file = hsr_arg_exists(r,"file") ;
	e->file = url_arg_to_file(file,e->file) ;
	e->line = hsr_arg_exists(r,"line") ;
	e->search = hsr_arg_exists(r,"search") ;
	e->expr = hsr_arg_exists(r,"expr") ;
	if (e->file) {
	    inserver_e(e) ;
	    }
	return 0 ;
	}
    case HPM_CONTENT_SEND:
	break ;
    case HPM_STATIC_CREATE:
	conf_try_all(".") ;
	break ;
    default:
	break ;
	}
    return(hpf_nocontent_ok(r,m,a)) ;
    }

/* ================================================================ */
static int exe_emacs(MT *exe,MT *cmd,char *arg)
{
    char	*xarg ;
    mtputs(exe,conf.emacs_exe) ;
    mtputs(cmd,"emacsclientw -n -e") ;
    if ((xarg = strchr(arg,'?'))) {
	*xarg++ = 0 ;
	}
    filename_slashify(arg,'/') ;
{
    int scheme = -1 ;
    MT file[1] ;
    MTALLOCA(file,1024) ;
    if (!memcmp_icase(arg,"http:",5)) {scheme = SCHEME_HTTP ; arg += 5 ;}
    unescape(file,arg) ;
    if (scheme == SCHEME_HTTP) url_to_file(file) ;
    mtprintf(cmd," \"(find-file \\\"%s\\\")\"",file->s) ;
    }
    if (xarg) {
	/* ? is line number */
	if (*xarg == '?') {
	    mtprintf(cmd," \"(goto-line %s)\"",xarg+1) ;
	   }
	/* ~ is search pattern */
	if (*xarg == '~') {
	    mtprintf(cmd," (bob) \"(rsf \\\"%s\\\")\"",xarg+1) ;
	   }
	}
    return(1) ;
    }

/* ================================================================ */
/* ================================================================ */
static int exe_explore(MT *exe,MT *cmd,char *arg)
{
    MTADVANCE(exe,GetWindowsDirectory(exe->s,MTGS(exe))) ;
    mtputs(exe,"\\explorer.exe") ;
    filename_slashify(arg,'\\') ;
    mtputs(cmd,"explore /e,\"") ;
    unescape(cmd,arg) ;
    mtputs(cmd,"\"") ;
    return(1) ;
    }

/* ================================================================ */
/* ================================================================ */
static int exe_shell_open(MT *exe,MT *cmd,char *arg)
{
    invoke_ShellExecute("Open",arg,"") ;
    return(0) ;
    }

static int exe_shell_edit(MT *exe,MT *cmd,char *arg)
{
    invoke_ShellExecute("Edit",arg,"") ;
    return(0) ;
    }

typedef struct {
    char	*verb ;
    char	*file ;
    char	*args ;
    } HLC_SHELL ;

/* ~~hpf(help => "Invoke shell open/edit on file")~~ */
static u32 hpf_shell(HSR *r,int m,u32 a)
{
    HLC_SHELL	*lc = (typeof(lc)) hsr_qlc(r,sizeof(HLC_SHELL)) ;
    switch (m) {
    case HPM_URL_MATCH:
	if (url_match(r,"/open")) {
	    lc->verb = "Open" ;
	    return 1 ;
	    }
	if (url_match(r,"/edit")) {
	    lc->verb = "Edit" ;
	    return 1 ;
	    }
	return(0) ;
    case HPM_INTERNAL: {
	lc->file = hsr_arg_exists(r,"file") ;
	lc->args = hsr_arg_exists(r,"args") ;
	if (lc->file) {
	    invoke_ShellExecute(lc->verb,lc->file,lc->args) ;
	    }
	return 0 ;
	}
    case HPM_CONTENT_SEND:
	break ;
    case HPM_STATIC_CREATE:
	conf_try_all(".") ;
	break ;
    default:
	break ;
	}
    return(hpf_nocontent_ok(r,m,a)) ;
    }

/* ================================================================ */

/* ================================================================ */
#include	"common.h"

EXETAB exetab[] = {
    {"emacs",	exe_emacs},
    {"explore",	exe_explore},
    {"open",	exe_shell_open},
    {"edit",	exe_shell_edit},
    {0},
    } ;

static int getexe(MT *exe,MT *cmd,char *arg)
{
    char	*file = arg ;
    EXETAB	*tab ;
    while (*file != ':') if (!*++file) errorfatal("No file") ;
    *file++ = 0 ;
    tab = exetab ;
    while (tab->tag) {
	if (!stricmp(tab->tag,arg)) {
	    return(tab->fun(exe,cmd,file)) ;
	    }
	tab++ ;
	}
    return(0) ;
    }

static int inserver(char *arg)
{
    MT		exe[1] ;
    MT		cmd[1] ;
    MTALLOCA(exe,1024) ;
    MTALLOCA(cmd,1024) ;
    if (getexe(exe,cmd,arg) == 0) return(0) ;
    return(invoke_CreateProcess(exe->s,cmd->s)) ;
    }

static int is_main(int argc, char **argv,char *mode)
{
    char *arg = argv[1] ;
    conf_try_all(mode) ;
    return inserver(arg) ;
}

/* ~# use mainmode ; #~ */
/* ~~mode("inserver",
   entry	=> "is_main",
   desc		=> "Application spawn server",
   )~~ */

#include	".gen/inserver.c"

#ifdef __cplusplus /*Z*/
}
#endif
