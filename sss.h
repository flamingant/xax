#ifdef __cplusplus /*Z*/
extern "C" {
#endif

#ifndef __sss_h
#define __sss_h

typedef struct {
    char	*public_ ;
    char	*private_ ;
    } SSH_KFP ;

#define AUTH_PUBLICKEY		1
#define AUTH_PASSWORD		2
#define AUTH_KEYBOARD		4

#ifndef __mt_h
#include	<pu/mt.h>
#endif

#ifndef __sock_h
#include	"sock.h"
#endif

typedef struct {
    int		magic ;
    char	*host ;
    char	*user ;
    char	*password ;
    SSH_KFP	*kfp ;
    int		state ;
    MT		*error ;
    int		sock ;
    int		auth_try ;
    int		auth_force ;
    LIBSSH2_SESSION *session ;
    LIBSSH2_CHANNEL *channel ;
    } SSS ;

extern SSH_KFP kfp_nick_boo[] ;

#define SSS_FAIL_SOCKET			-1
#define SSS_FAIL_CONNECT		-2
#define SSS_FAIL_AUTHENTICATION		-3
#define SSS_FAIL_SESSION		-4
#define SSS_FAIL_CHANNEL		-5
#define SSS_FAIL_SHELL			-6
#define SSS_FAIL_HANDSHAKE		-7

#define SSS_FAIL_FATAL			-9999

extern int libssh2_init_assert(void) ;
extern void libssh2_init_assert_or_die(void) ;

extern void sss_init(SSS *s) ;
extern void sss_init_once(SSS *s) ;

extern int sss_magic_check(SSS *s) ;
extern int sss_is_open(SSS *s) ;

extern void sss_error(SSS *s,char *fmt,...) ;

extern void sss_error_last(SSS *s,char *fmt) ;
extern char *sss_error_message(SSS *s) ;

extern int sss_authenticate(SSS *s) ;

extern int sss_arg_read(SSS *s,int argc, char *argv[]) ;

/* ================================================================ */
/* SCP */
#ifdef SSH_WANT_SCP

typedef struct {
    struct {
	struct stat stat ;
	char	*name ;
	} remote ;
    struct {
	struct stat stat ;
	char	*name ;
	int	fh ;
	} local ;
    } SCPS ;

extern int sss_scp_open(SSS *s) ;
extern void sss_scp_close(SSS *s) ;
extern void sss_scp_channel_close(SSS *s,SCPS *cps) ;
extern int sss_scp_channel_open_recv(SSS *s,SCPS *cps) ;
extern int sss_scp_read_loop(SSS *s,SCPS *cps) ;

#endif
/* ================================================================ */
/* SSH */

extern int sss_shell_open(SSS *s) ;
extern void sss_shell_close(SSS *s) ;

/* ================================================================ */
#ifdef SSH_INTERNAL

#define SSS_MAGIC	0x63636363

extern int sss_auth_methods(LIBSSH2_SESSION *session,char *user) ;

#endif

#endif

#ifdef __cplusplus /*Z*/
}
#endif
