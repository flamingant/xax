#ifdef __cplusplus /*Z*/
extern "C" {
#endif

#include	<libssh2.h>

#ifdef HAVE_WINSOCK2_H
# include <winsock2.h>
#endif

#include	<unistd.h>
#include	<arpa/inet.h>
#include	<sys/socket.h>

#include	<sys/types.h>
#include	<fcntl.h>
#include	<errno.h>
#include	<stdio.h>

#include	<netdb.h>

#include	"errors.h"

#include	<pu/exithook.h>

#include	<pu/mt.h>
#include	<stdlib.h>
#include	<stdarg.h>

#define SSH_WANT_SCP

#include	"sss.h"

static struct {
    SSS		s_feral[1] ;
    } g = {{}} ;

/* ================================================================ */
static int feral_scp_init(void)
{
    SSS		*s = g.s_feral ;
    if (sss_is_open(s)) return 0 ;
    sss_init(s) ;
    s->host = "erebus.feralhosting.com" ;
    s->user = "earwig" ;
    s->kfp = kfp_nick_boo ;
    if (libssh2_init_assert() < 0) return -1 ;

    if (sss_scp_open(s) == -1) return -1 ;
    exithook_install((EXITHOOK) sss_scp_close,s) ;
    return 0 ;
    }


static int torrent_download_scp(SSS *s,char *remote,char *local)
{
    int		rc ;
    SCPS	cps[1] ;

    cps->remote.name = remote ;
    cps->local.name = local ;
    if ((rc = sss_scp_channel_open_recv(s,cps)) != 0) goto done_1 ;
    rc = sss_scp_read_loop(s,cps) ;
    sss_scp_channel_close(s,cps) ;
done_1:
    return rc;
}

extern void feral_torrent_download(char *hash)
{
    char	*remote ;
    char	*local ;
    MT		u[1] ;
    MTALLOCA(u,1024) ;
    remote = u->c ;
    mtprintf(u,"/media/sdb1/home/earwig/.config/deluge/state/%s.torrent",hash) ;
    mtputc(u,0) ;
    local = u->c ;
    mtprintf(u,"data/torrents/%s.torrent",hash) ;
    mtputc(u,0) ;
    u->s = u->c ;
{
    SSS		*s = g.s_feral ;
    feral_scp_init() ;
    s->error = u ;
    torrent_download_scp(s,remote,local) ;
    }
    }

#ifdef __cplusplus /*Z*/
}
#endif
