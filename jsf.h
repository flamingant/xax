#ifdef __cplusplus /*Z*/
extern "C" {
#endif

#ifndef __jsf_h
#define __jsf_h	1

#ifndef JANSSON_H
#include	<jansson.h>
#endif

#include	"sf.h"

typedef struct {
    char	*name ;
    void	(*free)(SF *,void *) ;
    int		(*read)(SF *,void *,json_t *) ;
    } JSFT ;

extern JSFT jsft_charp[] ;
extern JSFT jsft_int[] ;
extern JSFT jsft_int64[] ;
extern JSFT jsft_bool[] ;
extern JSFT jsft_double[] ;
extern JSFT jsft_ENUM_TORRENTSTATE[] ;

extern JSFT *sft_jsft(SFT sft) ;

extern void jsf_free(SF *sf,void *p) ;
extern void jsf_read(SF *sf,void *p,json_t *o) ;

#ifndef __mt_h
#include	<pu/mt.h>
#endif

extern json_t *json_decode(MT *mti) ;

typedef char *(*JOSFUN)(const char *,json_t *,void *) ;
extern char *json_object_scan(json_t *o,JOSFUN fun,void *a) ;

typedef void (*JOIFUN)(const char *,json_t *,void *) ;
extern void json_object_iterate(json_t *o,JOIFUN fun,void *a) ;

extern void json_array_iterate(json_t *o,void (*fun)(json_t *,void *),void *a) ;

#ifndef __iterator_h
#include	<pu/iterator.h>
#endif

extern void json_object_iterator_open(ITERATOR *it,json_t *root,void *extra) ;
extern void json_array_iterator_open(ITERATOR *it,json_t *root,void *extra) ;

extern void json_iterator_destroy(ITERATOR *it) ;

#endif

#ifdef __cplusplus /*Z*/
}
#endif
