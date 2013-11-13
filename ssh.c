#ifdef __cplusplus /*Z*/
extern "C" {
#endif

#include	"ssh_h.h"

#include	<sys/types.h>
#include	<fcntl.h>
#include	<errno.h>
#include	<stdio.h>
#include	<ctype.h>
#include	<stdlib.h>

#include	<netdb.h>

#include	<stdarg.h>

#include	<pu/mt.h>

#include	"errors.h"

#define SSH_INTERNAL	1
#include	"sss.h"

#ifdef __CYGWIN__
#define EDRIVE	"//boo/e"	
#else
#define EDRIVE	"/vom/e"	
#endif

#define HOME	EDRIVE "/home/nick"	

SSH_KFP kfp_nick_boo[] = {
    HOME "/.ssh/id_rsa.pub",
    HOME "/.ssh/id_rsa",
    } ;

extern int socket_errno(void)
{
#ifdef WIN32
  return (int)WSAGetLastError();
#else
  return errno;
#endif
    }

extern int socket_close(int sock)
{
#ifdef WIN32
    return(closesocket(sock)) ;
#else
    return(close(sock)) ; 
#endif
    }

extern void sss_init(SSS *s)
{
    memset(s,0,sizeof(SSS)) ;
    s->magic = SSS_MAGIC ;
    }

extern void sss_init_once(SSS *s)
{
    if (s->magic != SSS_MAGIC)
	sss_init(s) ;
    }

extern int sss_is_open(SSS *s)
{
    return (s->magic == SSS_MAGIC) && (s->sock != 0) ;
    }

extern int sss_magic_check(SSS *s)
{
    if (s->magic != SSS_MAGIC) {
	errorfatal("Must not use an unprepared SSS structure") ;
	return(SSS_FAIL_FATAL) ;
	}
    return(0) ;
    }

extern int sss_auth_methods(LIBSSH2_SESSION *session,char *user)
{
    char *userauthlist;
    int	auth = 0 ;
    userauthlist = libssh2_userauth_list(session, user, strlen(user));

    if (strstr(userauthlist, "password") != NULL) {
        auth |= AUTH_PASSWORD ;
    }
    if (strstr(userauthlist, "keyboard-interactive") != NULL) {
        auth |= AUTH_KEYBOARD ;
    }
    if (strstr(userauthlist, "publickey") != NULL) {
        auth |= AUTH_PUBLICKEY ;
    }
    return(auth) ;
    }

static void kbd_callback(const char *name, int name_len,
                         const char *instruction, int instruction_len,
                         int num_prompts,
                         const LIBSSH2_USERAUTH_KBDINT_PROMPT *prompts,
                         LIBSSH2_USERAUTH_KBDINT_RESPONSE *responses,
                         void **abstract)
{
}

extern int sss_authenticate(SSS *s)
{
    s->auth_try = sss_auth_methods(s->session,s->user) ;;

    if (s->auth_force && (s->auth_try & s->auth_force))
	s->auth_try = s->auth_force ;

    if (s->auth_try & AUTH_PUBLICKEY) {
        if (!libssh2_userauth_publickey_fromfile(s->session, s->user, 
						 s->kfp->public_,s->kfp->private_,
						 s->password)) {
	    goto authenticated ;
        }
    }
    if (s->auth_try & AUTH_PASSWORD && s->password) {
        if (!libssh2_userauth_password(s->session, s->user, s->password)) {
 	    goto authenticated ;
        }
    }
    if (s->auth_try & AUTH_KEYBOARD) {
        if (!libssh2_userauth_keyboard_interactive(s->session, s->user,
                                                  &kbd_callback) ) {
	    goto authenticated ;
	}
	}
    sss_error(s,"All specified authentication methods (%d) failed",s->auth_try) ;
    return SSS_FAIL_AUTHENTICATION ;
authenticated:
    return 0 ;
    }

/* ================================================================ */

#include	<pu/exithook.h>

static void g_libssh2_exit(void)
{
    libssh2_exit();
    }

extern int libssh2_init_assert(void)
{
    static int state = 0 ;
    int		rc ;
    if (state > 0) return(state) ;
#ifdef WIN32
    WSADATA wsadata;
    WSAStartup(MAKEWORD(2,0), &wsadata);
#endif
    rc = libssh2_init (0);
    if (rc != 0) {
        errorshow("libssh2 initialization failed (%d)\n", rc);
        return -1 ;
    }
    exithook_install((EXITHOOK) g_libssh2_exit,0) ;
    state = 1 ;
    return(state) ;
    }

extern void libssh2_init_assert_or_die(void)
{
    if (libssh2_init_assert() < 0)
	errorfatal("... cannot continue\n") ;
    }

extern char *sss_error_message(SSS *s)
{
    if (s->error && s->error->s) return s->error->s ;
    return("") ;
    }

extern void sss_error(SSS *s,char *fmt,...)
{
    va_list	va ;
    if (!s->error) return ;
    va_start(va,fmt) ;
    mtvprintf(s->error,fmt,va) ;
    }

extern void sss_error_last(SSS *s,char *fmt)
{
    char *msg ;
    int len = 1024 ;
    int e = libssh2_session_last_error(s->session,&msg,&len,0) ;
    sss_error(s,fmt,msg,e) ;
    }

/* ================================================================ */
#if defined(NO_TESTCODE_AT_ALL) || defined(NO_TESTCODE_SSH) 
#else
/* ================================================================ */
/* =========================== TEST CODE ========================== */
#include	"arg.h"

static int long_arg_fun(char *name,char *value,void *a0)
{
    return(ASF_ARGIGNORED) ;
    }

extern int sss_arg_read(SSS *s,int argc, char *argv[])
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
	    case 'i':
		s->auth_force = AUTH_KEYBOARD ; 
		goto eat ;
	    case 'k':
		s->auth_force = AUTH_PUBLICKEY ; 
		goto eat ;
	    case 'p':
		s->auth_force = AUTH_PASSWORD ; 
		goto eat ;
	    default:
		break ;
	    }
	    }
	continue ;
eat:
	argv[i] = 0 ; 
	}
    argc = arg_compress(argc,argv) ;
    s->host = argv[1] ;
    s->user = argv[2] ;
    s->password = argv[3] ;
    argv[1] = argv[2] = argv[3] = 0 ;
    argc = arg_compress(argc,argv) ;
    return argc ;
}

#endif

#ifdef __cplusplus /*Z*/
}
#endif
