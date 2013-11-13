#ifdef __cplusplus /*Z*/
extern "C" {
#endif

#ifndef __http_h
#define __http_h	1

/* hsr ---> Http Server Request */

typedef struct struct_HSR HSR ;

typedef u32 (*HPF)(HSR *,int,u32) ;

typedef struct struct_HPO HPO ;

struct struct_HPO {
    HPF		f ;
    char	*name ;
    char	*desc ;
    char	*help ;
    } ;

/* ================================================================ */
#ifndef __uf_h
#include	"uf.h"
#endif

#ifndef __regex_h
#include	<pu/regex.h>
#endif

typedef struct {
    int		port ;
    int		sockfd ;
    UF		*uf ;
    int		state ;
    } HSS ;

#define HSS_SOCK_FAIL	0x01
#define HSS_BIND_FAIL	0x02

typedef struct {
    char	*name ;
    char	*value ;
    } QSE ;			/* ---> query string element */

#define MAXQLC		16

struct struct_HSR {
    HSS		*hss ;
    HPO		*hpo ;
    struct {
	HPF	hpf ;
	} _ ;
    int		csock ;
    MT		request[1] ;
    int		state ;
    int		method ;
    MT		url[1] ;
    MT		args[1] ;
    void	*lc ;
    char	*content_type ;
    struct {
	int	n ;
	QSE	*p ;
	MT	mt[1] ;
	} qs ;
    struct {
	int 	status ;
	} http ;
    char	qlc[MAXQLC] ;
    } ;

extern void *hsr_qlc(HSR *r,int size) ;
extern void hsr_qlc_set(HSR *,int) ;

extern void hsr_lc_free_safe(HSR *) ;
extern void hsr_lc_malloc(HSR *,int) ;

extern int url_match(HSR *r,char *s) ;

#define HM_GET		0
#define HM_POST		1
#define HM_HEAD		2

#define HSR_REQUEST_COMPLETE	0x01
#define HSR_REQUEST_FAIL	0x10
#define HSR_ACCEPT_FAIL		0x20
#define HSR_WRITE_FAIL		0x40

#define HSR_FAIL_MASK		(HSR_REQUEST_FAIL | HSR_ACCEPT_FAIL | HSR_WRITE_FAIL)

/* ================================================================ */
/* hpf ---> http page function - for delivering pages from server */

extern u32 hpf_generic_ok(HSR *r,int m,u32 a) ;
extern u32 hpf_nocontent_ok(HSR *r,int m,u32 a) ;

extern u32 hpf_null(HSR *r,int m,u32 a) ;

extern HPO hpo_not_found[] ;

/* ================================================================ */
#define HPM_URL_MATCH		0
#define HPM_HEADER_SEND		1
#define HPM_CONTENT_SEND	2
#define HPM_INTERNAL		3

#define HPM_CREATE		4
#define HPM_DESTROY		5
#define HPM_SILENT		6	/* execute for side-effects only */

#define HPM_TORRENT_APPLY_OK	7	/* can take a torrent hash parameter */

#define HPM_STATIC_CREATE	8
#define HPM_STATIC_DESTROY	9

#define HPM_GET_NAME		10
#define HPM_GET_DESC		11
#define HPM_GET_HELP		12

#define HPM_CAN_VISIT		13
#define HPM_REQUEST_DONE	14

extern void hsr_response_hdr(HSR *r) ;

extern void hsr_put(HSR *r,char *pb,int cb) ;
extern void hsr_puts(HSR *r,char *s) ;
extern void hsr_printf(HSR *r,char *fmt,...) ;

extern void hsr_close(HSR *r) ;

extern char *hsr_arg_exists(HSR *r,char *name) ;
extern char *hsr_arg_or_default(HSR *r,char *name,char *def) ;
extern char *hsr_arg_or_toggle(HSR *r,char *name) ;

extern char *hsr_arg_int(HSR *r,char *name,int *) ;

extern void hsr_execute(char *s) ;

extern HPO *hsr_bind_url(HSR *r,char *url) ;

extern u32 hsr_osend(HSR *r,int m,u32 a) ;

#ifdef __vcf_h
extern void hsr_vcf_pm(HSR *r,char *fmt,VCFFUN fun,void *c,int m) ;
extern void hsr_vcf_element_state(HSR *r,VCFFUN fun,void *c,char *element) ;
#endif

#endif

#ifdef __cplusplus /*Z*/
}
#endif
