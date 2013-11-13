#ifdef __cplusplus /*Z*/
extern "C" {
#endif

#include	<string.h>
#include	"talloc.h"

#include	<pu/type.h>

#include	"errors.h"
#include	"jsf.h"

static void jsft_charp_free(SF *f,void *p)
{
    char	**pp = (char **) SF_P(f,p) ;
    if (*pp) {free(*pp) ; *pp = 0 ;}
    }

static int jsft_charp_read(SF *f,void *p,json_t *o)
{
    char	**pp = (char **) SF_P(f,p) ;
    if (o->type == JSON_STRING)
	*pp = strdup(json_string_value(o)) ;
    return 0 ;
    }

JSFT jsft_charp[] = {"charp",jsft_charp_free,jsft_charp_read} ;

/* ================================================================ */
static void jsft_int_free(SF *f,void *p)
{
    }

static int jsft_int_read(SF *f,void *p,json_t *o)
{
    int		*fp = (int *) SF_P(f,p) ;
    if (o->type == JSON_INTEGER)
	*fp = json_integer_value(o) ;
    return 0 ;
    }

JSFT jsft_int[] = {"int",jsft_int_free,jsft_int_read} ;

/* ================================================================ */
static void jsft_int64_free(SF *f,void *p)
{
    }

static int jsft_int64_read(SF *f,void *p,json_t *o)
{
    u64		*fp = (u64 *) SF_P(f,p) ;
    if (o->type == JSON_INTEGER)
	*fp = json_integer_value(o) ;
    return 0 ;
    }

JSFT jsft_int64[] = {"int64",jsft_int64_free,jsft_int64_read} ;

/* ================================================================ */
static void jsft_bool_free(SF *f,void *p)
{
    }

static int jsft_bool_read(SF *f,void *p,json_t *o)
{
    int		*fp = (int *) SF_P(f,p) ;
    if (o->type == JSON_TRUE)
	*fp = 1 ;
    if (o->type == JSON_FALSE)
	*fp = 0 ;
    return 0 ;
    }

JSFT jsft_bool[] = {"bool",jsft_bool_free,jsft_bool_read} ;

/* ================================================================ */
static void jsft_double_free(SF *f,void *p)
{
    }

static int jsft_double_read(SF *f,void *p,json_t *o)
{
    double *fp = (double *) SF_P(f,p) ;
    if (o->type == JSON_REAL)
        *fp = json_real_value(o) ;
    return 0 ;
    }

JSFT jsft_double[] = {"double",jsft_double_free,jsft_double_read} ;

/* ================================================================ */
static void jsft_ENUM_TORRENTSTATE_free(SF *f,void *p)
{
    }

static char *torrent_state_name(ENUM_TORRENTSTATE s)
{
    switch (s) {
    case paused:	return("Paused") ;
    case downloading:	return("Downloading") ;
    case seeding:	return("Seeding") ;
    default:		return("*invalid*") ;
    }
    }
    
static int jsft_ENUM_TORRENTSTATE_read(SF *f,void *p,json_t *o)
{
    ENUM_TORRENTSTATE *fp = (ENUM_TORRENTSTATE *) SF_P(f,p) ;
    if (o->type == JSON_STRING) {
	const char *s = json_string_value(o) ;
	if (!strcmp(s,"Seeding")) 
	     *fp = seeding ;
	else if (!strcmp(s,"Downloading"))
	     *fp = downloading ;
	else if (!strcmp(s,"Paused"))
	     *fp = paused ;
	else *fp = invalid ;
	fflush(stdout) ;
	}
    return 0 ;
    }

JSFT jsft_ENUM_TORRENTSTATE[] = {"ENUM_TORRENTSTATE",jsft_ENUM_TORRENTSTATE_free,jsft_ENUM_TORRENTSTATE_read} ;

/* ================================================================ */
extern JSFT *sft_jsft(SFT sft)
{
    switch(sft) {
    case SFT_CHARP:	return(jsft_charp) ;
    case SFT_INT:	return(jsft_int) ;
    case SFT_INT64:	return(jsft_int64) ;
    case SFT_BOOL:	return(jsft_bool) ;
    case SFT_DOUBLE:	return(jsft_double) ;
    case SFT_ENUM_TORRENTSTATE:	return(jsft_ENUM_TORRENTSTATE) ;
    default:		errorfatal("unknown SFT %d\n",sft) ;
    }
    return(0) ;
    }
    
extern void jsf_free(SF *sf,void *p)
{
    JSFT *j = sft_jsft(sf->sft) ;
    j->free(sf,p) ;
    }

extern void jsf_read(SF *sf,void *p,json_t *o)
{
    JSFT *j = sft_jsft(sf->sft) ;
    j->read(sf,p,o) ;
    }

/* ================================================================ */
extern void json_array_iterate(json_t *o,void (*fun)(json_t *,void *),void *a)
{
    size_t	n = json_array_size(o) ;
    size_t	i ;
    json_t *value ;
    for (i = 0 ; i < n ; i++) {
	value = json_array_get(o,i) ;
	fun(value,a) ;
    }
    }

extern void json_object_iterate(json_t *o,JOIFUN fun,void *a)
{
    const char *key;
    json_t *value;
    key = json_object_iter_key(json_object_iter(o)) ;
    while (key && (value = json_object_iter_value(json_object_key_to_iter(key)))) {
	fun((char *) key,value,a) ;
        key = json_object_iter_key(json_object_iter_next(o,json_object_key_to_iter(key))) ;
	}
    }

extern char *json_object_scan(json_t *o,JOSFUN fun,void *a)
{
    char *r ;
    const char *key;
    json_t *value;
    key = json_object_iter_key(json_object_iter(o)) ;
    while (key && (value = json_object_iter_value(json_object_key_to_iter(key)))) {
	if ((r = fun(key,value,a))) return(r) ;
        key = json_object_iter_key(json_object_iter_next(o,json_object_key_to_iter(key))) ;
    }
    return(0) ;
}

extern json_t *json_decode(MT *mti)
{
    json_t *root;
    json_error_t error;
    root = json_loads(mti->s, 0, &error) ;
    return root ;
 }

/* ================================================================ */
#include	<pu/iterator.h>

extern void json_iterator_destroy(ITERATOR *it)
{
    free(it->context.v) ;
    }

typedef struct {
    json_t	*root ;
    const char	*key ;
    int		i ;
    int		n ;
    void	*extra ;
} JSON_OBJECT_ITERATOR_CONTEXT ;

extern u32 json_object_iterator_fun(ITERATOR *it,int m,u32 a)
{
    JSON_OBJECT_ITERATOR_CONTEXT *c = (typeof(c)) it->context.v ;
    switch (m) {
    case ITM_QUERY_FEATURE:
	return(1 << ITM_COUNT_TOTAL |
	       1 << ITM_COUNT_LEFT |
	       1 << ITM_REWIND |
	       0) ;
    case ITM_OPEN:
	c->n = json_object_size(c->root) ;
	goto ITM_REWIND ;
    case ITM_REWIND: ITM_REWIND:
	c->i = 0 ;
	c->key = json_object_iter_key(json_object_iter(c->root)) ;
	return(0) ;
    case ITM_COUNT_TOTAL:
	return(c->n) ;
    case ITM_COUNT_LEFT:
	return(c->n - c->i) ;
    case ITM_GET_NEXT: {
	json_t *value;
	if (!(value = json_object_iter_value(json_object_key_to_iter(c->key)))) {
	    return 0 ;
	    }
	*((u32 *) a) = (u32) value ;
	c->i++ ;
	c->key = json_object_iter_key(json_object_iter_next(c->root,json_object_key_to_iter(c->key))) ;
	return(1) ;
	}
    }
    return(0) ;
    }
    
extern void json_object_iterator_open(ITERATOR *it,json_t *root,void *extra)
{
    JSON_OBJECT_ITERATOR_CONTEXT *c = tscalloc(1,JSON_OBJECT_ITERATOR_CONTEXT) ;
    c->root = root ;
    c->extra = extra ;
    iterator_open(it,json_object_iterator_fun,c) ;
    }

/* ================================================================ */
typedef struct {
    json_t	*root ;
    int		i ;
    int		n ;
    void	*extra ;
} JSON_ARRAY_ITERATOR_CONTEXT ;

extern u32 json_array_iterator_fun(ITERATOR *it,int m,u32 a)
{
    JSON_ARRAY_ITERATOR_CONTEXT *c = (typeof(c)) it->context.v ;
    switch (m) {
    case ITM_QUERY_FEATURE:
	return(1 << ITM_COUNT_TOTAL |
	       1 << ITM_COUNT_LEFT |
	       1 << ITM_REWIND |
	       0) ;
     case ITM_OPEN:
	c->n = json_array_size(c->root) ;
	c->i = 0 ;
	goto ITM_REWIND ;
    case ITM_COUNT_TOTAL:
	return(c->n) ;
    case ITM_COUNT_LEFT:
	return(c->n - c->i) ;
    case ITM_REWIND: ITM_REWIND:
	c->i = 0 ;
	return(0) ;
    case ITM_GET_NEXT:
	if (c->i >= c->n) return 0 ;
	*((u32 *) a) = (u32) json_array_get(c->root,c->i) ;
	c->i++ ;
	return(1) ;
   }
    return(0) ;
    }
    
extern void json_array_iterator_open(ITERATOR *it,json_t *root,void *extra)
{
    JSON_ARRAY_ITERATOR_CONTEXT *c = tscalloc(1,JSON_ARRAY_ITERATOR_CONTEXT) ;
    c->root = root ;
    c->extra = extra ;
    iterator_open(it,json_array_iterator_fun,c) ;
    }


#ifdef __cplusplus /*Z*/
}
#endif
