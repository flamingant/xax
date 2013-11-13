#ifdef __cplusplus /*Z*/
extern "C" {
#endif

#ifndef __dht_h
#define __dht_h	1

#define DHT_MAX		65536

/* dht_ ---> contraction of deluge_ht_ */

extern void param_add(MT *mt,char *name,char *value) ;
extern void param_add_u(MT *mt,char *name,char *value) ;

extern u32 tsi_fname_list_to_mask(char *names) ;

/* ================================================================ */
typedef u32 (*TSMF)(UF *,int,u32) ;

extern u32 null_tsmf(UF *uf,int m,u32 a) ;

#define TSM_REPLY_COMPLETE	0

extern UF *deluge_ht_create(UFF uff,char *post,void *cp) ;
extern UF *tod_deluge_ht_create(UFF uff,char *post,TOD *tod) ;

extern void torrent_alphabetize(TOD *tod) ;
extern void torrent_label(TOD *tod,char *label) ;

extern void thmi_pause(char *hash) ;
extern void thmi_remove(char *hash,char *params) ;

extern UF *dht_get_status_everything(TSMF) ;
extern UF *dht_get_status_filter_mask(TSMF,char *filter,int mask) ;
extern UF *dht_get_status_hash_mask(TSMF,char *hash,int mask) ;
extern UF *dht_get_status_hashlist_mask(TSMF f,char *hashlist,int mask) ;

extern void dht_relabel(char *label,char *style) ;

extern void dht_init(void) ;
extern void dht_onexit(void) ;

extern UF *dht_add_torrent_url(char *id) ;
extern UF *dht_add_torrent_url_i(int) ;

extern void dht_session_plugin(void) ;
extern void dht_session_unplug(void) ;

#include	"vcf.h"

#define RSM_START	VCF_SWITCH_ON
#define RSM_STOP	VCF_SWITCH_OFF
#define RSM_TOGGLE	VCF_SWITCH_TOGGLE
#define RSM_GETSTATE	VCF_GETINT
#define RSM_SETSTATE	VCF_SETINT

extern u32 dht_session_rsf(int,u32) ;

extern HT *ht_create_port(int port,char *method,char *host,char *page,char *post) ;
extern HT *ht_create(char *method,char *host,char *page,char *post) ;
extern HT *https_create(char *method,char *host,char *page,char *post) ;

extern u32 ht_uff(UF *f,int m,u32 a) ;
extern u32 dht_uff(UF *f,int m,u32 a) ;

extern int header_find(MD *mi,MD *mo,char *name) ;
extern int header_copy(MD *mi,MT *mto,char *name) ;
extern int ht_header_find(HT *ht,MD *mo,char *name) ;
extern int ht_header_copy(HT *ht,MT *mto,char *name) ;

#endif

#ifdef __cplusplus /*Z*/
}
#endif
