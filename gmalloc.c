#ifdef __cplusplus /*Z*/
extern "C" {
#endif

#define GMC_INTERNALS

#include	"gmalloc.h"
#include	<string.h>

/* ================================================================ */
extern void gmalloc_close(GMC *c)
{
    c->t->generic(c,GMM_CLOSE,0) ;
    free(c) ;
    }

/* ================================================================ */
extern void gfree(GMC *c,void *p)
{
    if (p)
	c->t->free(c,p) ;
    }

extern void *gmalloc(GMC *c,size_t size)
{
    return c->t->malloc(c,size) ;
    }

extern void *gcalloc(GMC *c,size_t n,size_t size)
{
    void *p = c->t->malloc(c,size * n) ;
    memset(p,0,size * n) ;
    return p ;
    }

extern void *gremalloc(GMC *c,void *p,size_t size)
{
    return c->t->realloc(c,p,size) ;
    }

extern char *gstrdup(GMC *c,char *s)
{
    int		n = strlen(s)+1 ;
    char	*p = (char *) c->t->malloc(c,n) ;
    memcpy(p,s,n) ;
    return p ;
    }

/* ================================================================ */
extern long gmc_generic_null(GMC *c,int m,long a)
{
    return 0 ;
    }

/* ================================================================ */
static long std_malloc_generic(GMC *c,int m,long a)
{
    return gmc_generic_null(c,m,a) ;
    }

extern void std_malloc_free(GMC *c,void *p)
{
    if (p) free(p) ;
    }

extern void *std_malloc_malloc(GMC *c,size_t size)
{
    return malloc(size) ;
    }

extern void *std_malloc_realloc(GMC *c,void *p,size_t size)
{
    return realloc(p,size) ;
    }

GMCT gmct_std_malloc[] = {
    std_malloc_generic,
    std_malloc_malloc,
    std_malloc_realloc,
    std_malloc_free
    } ;

GMC gmc_std_malloc[] = {gmct_std_malloc,0} ;
/* ================================================================ */
#include	<pu/mt.h>
#include	"errors.h"

extern GMCT gmct_mt_malloc[] ;

#define GMDMT(c)	((MT *) (c)->d)

extern void mt_malloc_init(GMC *c,int size)
{
    ANZF(sizeof(MT) < sizeof(c->_d_)) ;
    c->d = c->_d_ ;
    c->t = gmct_mt_malloc ;
    mtmalloc(GMDMT(c),size) ;
    }

extern GMC *mt_malloc_open(int size)
{
    GMC *c = (GMC *) malloc(sizeof(GMC)) ;
    mt_malloc_init(c,size) ; 
    return c ;
    }

static long mt_malloc_generic(GMC *c,int m,long a)
{
    switch(m) {
    case GMM_CLOSE:
	mtfree(GMDMT(c)) ;
	return 0 ;
	}
    return gmc_generic_null(c,m,a) ;
    }

extern void mt_malloc_free(GMC *c,void *p)
{
    /* no free here */
    }

extern void *mt_malloc_malloc(GMC *c,size_t size)
{
    MT		*mt = GMDMT(c) ;
    void	*r ; 
    if (size > MTGapSize(mt)) {
	errorshow("you haven't allocated enough space") ;
	return(0) ;
	}
    r = mt->c ;
    mt->c += size ;
    return r ;
    }

extern void *mt_malloc_realloc(GMC *c,void *p,size_t size)
{
    return mt_malloc_malloc(c,size) ;
    }

GMCT gmct_mt_malloc[] = {
    mt_malloc_generic,
    mt_malloc_malloc,
    mt_malloc_realloc,
    mt_malloc_free
    } ;

/* ================================================================ */

#ifdef __cplusplus /*Z*/
}
#endif
