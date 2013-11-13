#ifdef __cplusplus /*Z*/
extern "C" {
#endif

#ifndef __ma_h
#define __ma_h	1

#ifndef __uf_h
#include	"uf.h"
#endif

/* ================================================================ */
/* the choice is either to 
   a) have the base class components as a member of the subclass (inner base)
      the subclass calls the base class methods as required.

   b) have the base class hold the context of the subclass (outer base)
      the base class calls the subclass methods in addition to its own

   The base class function receives the same context pointer as the
   subclass, so the location of the base class would always have to
   be in the same place, but the compiler cannot enforce this.

   Therefore approach b) is better for that reason.
   However, we normally want class functions to receive the context
   of the subclass as we want the fact that it is a subclass
   be transparent (although we know it isn't because we end up
   passing some messages to the base class).

   A drawback of inner base is that you have to know when processing
   by the base class is always required. If the subclass returns without
   calling the base class method, then you will get problems that are
   quite hard to detect.

   So, approach a) can have its upside, although the downside is
   as described, the base class context must always be in the same
   place.

   */
/* ================================================================ */
#include	<arpa/inet.h>

typedef struct struct_SSLHT SSLHT ;

struct __struct_HT {
    UF		*uf ;
    int		state ;
    int		fail ;
    int		sock ;
    int		port ;
    char	*method ;
    char	*host ;
    char	*page ;
    char	*post ;
    struct in_addr ipaddr ;
    MT		request[1] ;
    MT		response[1] ;
    int		http_status ;
    char	*error ;
    struct {
	int	obody_s ;
	int	obody_c ;
	} rsp ;
    int		seq ;
    void	*sc ;	/* subtype context */
    SSLHT	*ssl ;
    char	*cookie ;
    } ;

#define HTS_SOCK		0x0001
#define HTS_GET_IP		0x0002
#define HTS_CONNECT		0x0004
#define HTS_SSL_CONNECT		0x0008

#define HTS_REQUEST		0x0100
#define HTS_RESPONSE		0x0200

#define HT_SETFAIL(h,i)		((h)->fail |= (i))
#define HT_SETDONE(h,i)		((h)->state |= (i))
#define HT_ISNEED(h,i)		(!((h)->state & (i)))

/* ================================================================ */
#ifndef JANSSON_H
#include	<jansson.h>
#endif

typedef struct {
    MT	mtz[1] ;
    MT	mtu[1] ;
    json_t *j ;
    } RBP ;

extern void tlsi_accept_response(UF *uf,HT *ht,RBP *r) ;

#include	"tod.h"

extern HT *ht_create(char *method,char *host,char *page,char *post) ;

extern void feral_torrent_download(char *hash) ;

#endif

#ifdef __cplusplus /*Z*/
}
#endif
