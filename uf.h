#ifdef __cplusplus /*Z*/
extern "C" {
#endif

#ifndef __uf_h
#define __uf_h

#ifndef __type_h
#include	<pu/type.h>
#endif

typedef struct __struct_UF UF ;

typedef struct __struct_HT HT ;

typedef union {
    void	*v ;
    HT		*ht ;
#ifdef UFD_MEMBERS
UFD_MEMBERS
#endif
    } UFD ;

typedef u32 (*UFF)(UF *,int,u32) ;

/*
~# use decode ; #~
~~enum_decode("UFM")~~
*/

typedef enum __enum_UFM {
    UFM_CREATE,
    UFM_DESTROY,
    UFM_TYPENAME,
    UFM_DESCRIBE,
    UFM_TRACE_OK,
    UFM_MSG_NAME,
    UFM_ARG_DESCRIBE,

    UFM_QUEUE_ADD,
    UFM_READ,
    UFM_CONNECT,
    UFM_CONNECT_OK,
    UFM_SELECT_OK,
    UFM_WAKEUP,

    UFM_OPTION_TRY,
    UFM_GET_STATIC,

    UFM_PRE_NOTIFY,
    UFM_POST_NOTIFY,
    UFM_BUFFER_DRAIN,

    UFM_DISCONNECT_IND,

    UFM__STD_AFTERLAST,

    UFM__HT_FIRST	= 100,

    UFM_HT_RESPONSE_HDR	= UFM__HT_FIRST,
    UFM_HT_RESPONSE_BODY,
    UFM_HT_RESPONSE_JSON,
    UFM_HT_TORRENT_STATUS_GET,
    UFM_HT_NOTREADY,

    UFM_TORRENT_ADDED,

    UFM__HT_AFTERLAST,
    } UFM ;

/* note carefully fencepost issues */

#define UFM__COUNT_HT	(UFM__HT_AFTERLAST-UFM__HT_FIRST)
#define UFM__COUNT_STD	(UFM__STD_AFTERLAST)
#define UFM__COUNT	(UFM__COUNT_STD + UFM__COUNT_HT)

#define UFS_DESTROY	0x0001

typedef struct {
    u8		all ;
    u8		none ;
    u8		ufm[UFM__COUNT] ;
    } UF_TRACE ;

typedef struct __struct_UFQ UFQ ;

struct __struct_UFQ {
    int		m ;
    u32		a ;
    UFQ		*next ;
    } ;

struct __struct_UF {
    UFF		f ;
    UFD		d ;
    int		state ;
    UF		*parent ;
    UF_TRACE	trace ;
    struct {
	UFQ	*head ;
	UFQ	**tail ;
	} queue ;
    } ;

typedef struct {
    UF		*uf ;
    int	       	m ;
    u32		a ;
    } UFM_NOTIFY_ARG ;

extern UF *uf_create(UFF f,void *d,void *cp) ;
extern void uf_destroy(UF *uf) ;

extern void uf_socket_register(UF *uf,int fd) ;

extern u32 uf_send(UF *uf,int m,u32 a) ;
extern u32 uf_send_direct(UF *uf,int m,u32 a) ;

extern void uf_queue(UF *uf,int m,u32 a) ;
extern void uf_queue_tail(UF *uf,int m,u32 a) ;
extern void uf_unqueue_head(UF *uf) ;

extern u32 uf_parent_notify(UF *uf,int nm,int m,u32 a) ;

extern u32 null_uff(UF *f,int m,u32 a) ;

extern void uf_socket_unregister(UF *uf,int fd) ;

extern void ufs_cycle(void) ;
extern void ufs_loop(void) ;
extern int ufs_init(int argc,char **argv) ;
extern void ufs_destroy(void) ;
extern void ufs_rc_add(UF *uf) ;

typedef void (*UFMAPFUN)(UF *) ;
typedef void (*UFMAPFUN1)(UF *,u32) ;

typedef int (*UFSCANFUN)(UF *) ;
typedef int (*UFSCANFUN1)(UF *,u32) ;

#ifdef __rcons_h
extern void uf_map_rcons(RCONS *r,UFMAPFUN) ;
extern void uf_map1_rcons(RCONS *r,UFMAPFUN1,u32 a) ;

extern UF *uf_scan_rcons(RCONS *r,int (*fun)(UF *)) ;
extern UF *uf_scan1_rcons(RCONS *r,int (*fun)(UF *,u32),u32 a) ;
#endif

extern void ufs_quit(void) ;

extern void uf_trace(UF *uf,char *fmt,...) ;

/* ================================================================ */
/* 
   until now, there has been no storage associated with UFF functions.
   The idea was that everything would be provided through the UFF,
   including name.
   Every UFF must respond to a UFM_TYPENAME, but it didn't mean that
   any static space was reserved for the name in a structure.
   However, there are data elements that would normally always be
   handled the same way, so by default there can be a common
   structure which can be accessed from the default UFF functions
   in order that debug options can be set up ahead of creation
   of UF objects
*/

typedef struct {
    char	*name ;
    UF_TRACE	trace ;
    } UFSS ;		/* Standard Static */
	
/* ================================================================ */
#endif

#ifdef __cplusplus /*Z*/
}
#endif
