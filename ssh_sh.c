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

#include	<pu/mt.h>
#include	"errors.h"

#define SSH_INTERNAL	1

#include	"sss.h"

#include	"uf.h"

static void ssh_remote_output_sink(UF *uf,SSS *s)
{
    char inputbuf[BUFSIZ];
    int		rc ;
    do {
	rc = libssh2_channel_read(s->channel, inputbuf, BUFSIZ);
	printf("%s", inputbuf);
	fflush(stdout);
	memset(inputbuf, 0, BUFSIZ) ;
    } while (LIBSSH2_ERROR_EAGAIN != rc && rc > 0);
}

static u32 ssh_uff(UF *uf,int m,u32 a)
{
    SSS		*s = uf->d.v ;
    switch(m) {
    case UFM_DESTROY:
	sss_shell_close(s) ;
    case UFM_CREATE:
	break ;
    case UFM_TYPENAME:	return(mtputs((MT *) a,"ssh")) ;
    case UFM_TRACE_OK: return(0) ;
    case UFM_SELECT_OK:
	ssh_remote_output_sink(uf,s) ;
    default:
	break ;
	}
    return(null_uff(uf,m,a)) ;
    }

extern UF *ssh_uf_create(SSS *s)
{
    UF *uf = uf_create(ssh_uff,s,0) ;
    uf_socket_register(uf,s->sock) ;
    return uf ;
    }

/* ================================================================ */
extern void shell_send(SSS *s,char *data)
{
    int len = strlen(data) ;
    int bytes_write ;
    int bytes_write_total = 0;
    do {
	bytes_write = libssh2_channel_write(s->channel,data, len) ;
	bytes_write_total += bytes_write ;
	len -= bytes_write ;
	data += bytes_write ;
	} while (bytes_write != LIBSSH2_ERROR_EAGAIN
		 && bytes_write > 0
		 && len > 0) ;
    }

/* ================================================================ */
extern void sss_shell_close(SSS *s)
{
    if (s->channel) {
        libssh2_channel_free(s->channel) ;
        s->channel = NULL;
    }

    if (s->session) {
	libssh2_session_disconnect(s->session, "Normal Shutdown, Goodbye.");
	libssh2_session_free(s->session);
	s->session = 0 ;
    }

    if (s->sock) {
	socket_close(s->sock) ;
	s->sock = 0 ;
    }
}

extern int sss_shell_open(SSS *s)
{
    struct hostent *hent;
    unsigned long hostaddr;
    struct sockaddr_in sin;
    int rc ;

    if (sss_magic_check(s) < 0)
	return(SSS_FAIL_FATAL) ;

    if (!(s->sock = socket(AF_INET, SOCK_STREAM, 0))) {
        sss_error(s,"socket: returned zero") ;
        rc = SSS_FAIL_SOCKET ;
	goto fail ;
    }

    hent = gethostbyname(s->host) ;
    hostaddr = *((ulong *) *hent->h_addr_list) ;

    sin.sin_family = AF_INET;
    sin.sin_port = htons(22);
    sin.sin_addr.s_addr = hostaddr;
    rc = connect(s->sock,(struct sockaddr*)(&sin),sizeof(sin)) ;
    if (rc != 0) {
        sss_error(s,"connect: returned error %d",rc) ;
        rc = SSS_FAIL_CONNECT ;
	goto fail ;
    }

    if (!(s->session = libssh2_session_init())) {
        sss_error(s,"libssh2_session_init() failed to return valid session") ;
        rc = SSS_FAIL_SESSION ;
	goto fail ;
	}

    rc = libssh2_session_handshake(s->session, s->sock) ;
    if (rc != 0) {
        sss_error(s,"libssh2_session_handshake: returned error (%d)",rc) ;
        rc = SSS_FAIL_HANDSHAKE ;
	goto fail ;
    }

    if ((rc = sss_authenticate(s)) != 0) {
	goto fail ;
    }

    if (!(s->channel = libssh2_channel_open_session(s->session))) {
        sss_error(s,"libssh2_channel_open_session failed to return session") ;
	rc = SSS_FAIL_SESSION ;
	goto fail ;
    }

    /* See /etc/termcap for more terminal options */
    if (libssh2_channel_request_pty(s->channel, "dumb")) {
        sss_error(s,"libssh2_channel_request_pty failed to allocate pty") ;
        rc = SSS_FAIL_CHANNEL ;
	goto fail ;
    }

    /* Open a SHELL on that pty */
    if (libssh2_channel_shell(s->channel)) {
        sss_error(s,"libssh2_channel_shell failed to open shell") ;
        rc = SSS_FAIL_SHELL ;
	goto fail ;
    }

    libssh2_channel_set_blocking(s->channel, 0);

fail:
    return(rc) ;
    }

/* ================================================================ */
#if defined(NO_TESTCODE_AT_ALL) || defined(NO_TESTCODE_SSH_SH) 

extern void *mainmode_ssh_sh(char *mode) { return(0) ; }

#else
/* =========================== TEST CODE ========================== */
/* ================================================================ */
extern int sss_arg_read(SSS *s,int argc, char *argv[]) ;

static int sss_test(SSS *s,int argc, char *argv[])
{
    int rc ;
    libssh2_init_assert_or_die() ;

    if ((rc = sss_shell_open(s)) < 0) {
	fprintf (stderr, "sss_shell_open failed (%d) - %s\n",rc,sss_error_message(s)) ;
	return rc ;
	}

    ufs_init(argc,argv) ;
    ssh_uf_create(s) ;
    shell_send(s,"ls\n") ;
    ufs_loop() ;
    ufs_destroy() ;
    return 0 ;
}

static int sm_1(int argc, char *argv[])
{
    SSS		*s = calloc(1,sizeof(SSS)) ;
    MT		mterror[1] ;
    sss_init(s) ;
    argc = sss_arg_read(s,argc,argv) ;
    MTALLOCA(mterror,256) ;
    s->error = mterror ;

    s->kfp = kfp_nick_boo ;

    return sss_test(s,argc,argv) ;
    }

/* ================================================================ */
#include	<string.h>
#include	"arg.h"
#include	"mainmode.h"

static struct {
    char	*smode ;
    } g ;

static int ssh_sh_main(int argc, char *argv[],MMC *c)
{
    if (!(g.smode = arg_get_submode(argc,argv))) g.smode = "" ;
    argc = arg_compress(argc,argv) ;
    if (!strcmp(g.smode,"1")) return(sm_1(argc,argv)) ;
    return(sm_1(argc,argv)) ;
    }

/* ~# use mainmode ; #~ */
/* ~~mode("ssh_sh",
   desc		=> "ssh shell test mode",
   )~~ */

#include	".gen/ssh_sh.c"

#endif

#ifdef __cplusplus /*Z*/
}
#endif
