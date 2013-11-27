#ifdef __cplusplus /*Z*/
extern "C" {
#endif

/*
  scp will not work if the remote host outputs text in the
  .bashrc file during a non-interactive shell session.
  Not sure why this should be an issue, but it was observed
  doing a manual scp command from the shell that the remote
  host emitted "#host:"`hostname` which derailied the scp maybe
  because some other protocol specific response was expected

  We are not making the scp send function UF controlled yet
  1. because we aren't sure how select would work on file handles
  2. it isn't two way communication
  3. we may want to break the transfer into chunks but
     that can be done without select
 
  */

#include	"ssh_h.h"

#include	<sys/types.h>
#include	<fcntl.h>
#include	<errno.h>
#include	<stdio.h>

#include	<netdb.h>

#include	"errors.h"

#define SSH_WANT_SCP	1

#include	"sss.h"

/* ================================================================ */
#define SSS_BUFFER_SIZE		65536

#define SCP_FAIL_LOCAL_OPEN	-10

extern int sss_scp_read_loop(SSS *s,SCPS *cps)
{
    int		rc ;
    struct stat *stat = &cps->remote.stat ;
    off_t total_bytes_read = 0 ;
    int bytes_read ;
    int bytes_write = 0 ;

    if (!(cps->local.fh = open(cps->local.name,O_WRONLY | O_CREAT))) {
	sss_error(s,"Unable to open local file") ;
	rc = SCP_FAIL_LOCAL_OPEN ;
        goto shutdown ;
    }

    while (total_bytes_read < stat->st_size) {
        char filebuf[SSS_BUFFER_SIZE];
        int read_chunk_size = SSS_BUFFER_SIZE ;

        if((stat->st_size - total_bytes_read) < read_chunk_size) {
            read_chunk_size = stat->st_size - total_bytes_read;
        }

        bytes_read = libssh2_channel_read(s->channel, filebuf, read_chunk_size);
        if (bytes_read > 0) {
            bytes_write = write(cps->local.fh, filebuf, bytes_read) ;
        }
        else if (bytes_read < 0) {
	    sss_error_last(s,"libssh2_channel_read() failed - %s (%d)") ;
            break;
        }
        total_bytes_read += bytes_read ;
    }
    rc = 0 ;
shutdown:
    return rc ;
    }

extern int sss_scp_send_loop(SSS *s,SCPS *cps)
{
    int		rc ;
    struct stat *stat = &cps->local.stat ;
    off_t total_bytes_read = 0 ;
    int bytes_read ;
    int bytes_write = 0 ;

    if (!(cps->local.fh = open(cps->local.name,O_RDONLY))) {
	sss_error(s,"Unable to open local file") ;
	rc = SCP_FAIL_LOCAL_OPEN ;
        goto shutdown ;
    }

    while (total_bytes_read < stat->st_size) {
        char filebuf[SSS_BUFFER_SIZE];
        int read_chunk_size = SSS_BUFFER_SIZE ;

        if((stat->st_size - total_bytes_read) < read_chunk_size) {
            read_chunk_size = stat->st_size - total_bytes_read;
        }

        bytes_read = read(cps->local.fh, filebuf, read_chunk_size);
        if (bytes_read > 0) {
            bytes_write = libssh2_channel_write(s->channel, filebuf, bytes_read) ;
        }
        else if (bytes_read < 0) {
	    sss_error_last(s,"libssh2_channel_write() failed - %s (%d)") ;
            break;
        }
        total_bytes_read += bytes_read ;
    }
    rc = 0 ;
shutdown:
    return rc ;
    }

/* ================================================================ */
extern void sss_scp_close(SSS *s)
{
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

extern int sss_scp_open(SSS *s)
{
    struct hostent *hent;
    unsigned long hostaddr;
    struct sockaddr_in sin;
    int rc ;

    if (sss_magic_check(s) < 0)
	return(SSS_FAIL_FATAL) ;

    if (s->sock != 0) return 0 ;

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

    s->session = libssh2_session_init();
    if (!s->session) {
	sss_error(s,"libssh2_session_init() failed to return valid session") ;
	goto fail;
	}

    rc = libssh2_session_handshake(s->session, s->sock);
    if (rc != 0) {
        sss_error(s,"libssh2_session_handshake: returned error (%d)",rc) ;
        rc = SSS_FAIL_HANDSHAKE ;
    }

    if ((rc = sss_authenticate(s)) != 0) {
	goto fail ;
    }
    return 0 ;
fail:
    return rc ;
    }

/* ================================================================ */
extern void sss_scp_channel_close(SSS *s,SCPS *cps)
{
    if (cps->local.fh) {
	close(cps->local.fh) ;
	cps->local.fh = 0 ;
	}
    if (s->channel) {
	libssh2_channel_free(s->channel);
	s->channel = 0 ;
	}
}

extern int sss_scp_channel_open_recv(SSS *s,SCPS *cps)
{
    s->channel = libssh2_scp_recv(s->session, cps->remote.name, &cps->remote.stat) ;
    if (!s->channel) {
	sss_error_last(s,"Unable to open remote channel - %s (%d)") ;
        return SSS_FAIL_CHANNEL ;
    }
    return 0 ;
    }
    
extern int sss_scp_channel_open_send(SSS *s,SCPS *cps)
{
    libssh2_uint64_t size ;
    stat(cps->local.name,&cps->local.stat) ;
    size = cps->local.stat.st_size ;

    s->channel = libssh2_scp_send64(s->session,
				    cps->remote.name,
				    0644,
				    size,
				    cps->local.stat.st_ctime,
				    cps->local.stat.st_atime) ;

    if (!s->channel) {
	sss_error_last(s,"Unable to open remote channel - %s (%d)") ;
        return SSS_FAIL_CHANNEL ;
    }
    return 0 ;
    }
    
/* ================================================================ */
#if defined(NO_TESTCODE_AT_ALL) || defined(NO_TESTCODE_SSH_SCP) 

extern void *mainmode_ssh_scp(char *mode) { return(0) ; }

#else
/* =========================== TEST CODE ========================== */
/* ================================================================ */
extern void feral_torrent_download(char *hash) ;

static int sm_1(int argc, char *argv[])
{
    feral_torrent_download("00347e2e266cf98b8264f758963438cfbb85846a") ;
    feral_torrent_download("00779e937e6c127aa8b472fe6d0762908f284909") ;
    return 0 ;
}

/* ================================================================ */
static int sm_2(int argc, char *argv[])
{
    SCPS	cps[1] ;
    SSS		s[1] ;
    int		rc ;
    if (libssh2_init_assert() < 0) return -1 ;
    sss_init(s) ;
    argc = sss_arg_read(s,argc,argv) ;
    s->kfp = kfp_nick_boo ;
    cps->remote.name = argv[1] ;
    if (!(cps->local.name = argv[2])) cps->local.name = cps->remote.name ;
    if ((rc = sss_scp_open(s)) != 0) return rc ;
    if ((rc = sss_scp_channel_open_recv(s,cps)) != 0) goto done_1 ;
    rc = sss_scp_read_loop(s,cps) ;
    sss_scp_channel_close(s,cps) ;
done_1:
    sss_scp_close(s) ;
    return(rc) ;
    }

/* ================================================================ */
/* test send file */

#include	<stdlib.h>

static int sm_3(int argc, char *argv[])
{
    SCPS	cps[1] ;
    SSS		s[1] ;
    int		rc ;
    MT		error[1] ;
    MTALLOCA(error,1024) ;
    if (libssh2_init_assert() < 0) return -1 ;
    sss_init(s) ;
    s->error = error ;
    argc = sss_arg_read(s,argc,argv) ;
    s->kfp = kfp_nick_boo ;
    cps->remote.name = argv[1] ;
    if (!(cps->local.name = argv[2])) cps->local.name = cps->remote.name ;
    if ((rc = sss_scp_open(s)) != 0) return rc ;
    if ((rc = sss_scp_channel_open_send(s,cps)) != 0) goto done_1 ;
    rc = sss_scp_send_loop(s,cps) ;
    sss_scp_channel_close(s,cps) ;
done_1:
    sss_scp_close(s) ;
    return(rc) ;
}

/* ================================================================ */
#include	<string.h>
#include	"arg.h"
#include	"mainmode.h"

static struct {
    char	*smode ;
    } g ;

static int ssh_scp_main(int argc, char *argv[],MMC *c)
{
    if (!(g.smode = arg_get_submode(argc,argv))) g.smode = "3" ;
    argc = arg_compress(argc,argv) ;
    if (!strcmp(g.smode,"3")) return(sm_3(argc,argv)) ;
    if (!strcmp(g.smode,"2")) return(sm_2(argc,argv)) ;
    if (!strcmp(g.smode,"1")) return(sm_1(argc,argv)) ;
    return(sm_3(argc,argv)) ;
    }

/* ~# use mainmode ; #~ */
/* ~~mode("ssh_scp",
   desc		=> "ssh copy (scp) test mode",
   )~~ */

#include	".gen/ssh_scp.c"

#endif

#ifdef __cplusplus /*Z*/
}
#endif
