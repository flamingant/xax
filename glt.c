#ifdef __cplusplus /*Z*/
extern "C" {
#endif

#include	"glt.h"

extern GLT *glassq(GLT *t,GLT_CAR_T v)
{
    if (!t) return 0 ;
    for ( ; t->car || t->cdr ; t++)
	if (t->car == v) return t ;
    return 0 ;
    }

extern GLT *glrassq(GLT *t,GLT_CDR_T v)
{
    if (!t) return 0 ;
    for ( ; t->car || t->cdr ; t++)
	if (t->cdr == v) return t ;
    return 0 ;
    }

extern GLT *glassoc(GLT *t,GLT_CAR_T v,GLTCMP cmp)
{
    if (!t) return 0 ;
    if (!cmp) return glassq(t,v) ;
    for ( ; t->car || t->cdr ; t++)
	if (!cmp(t->car,v)) return t ;
    return 0 ;
    }

extern GLT *glrassoc(GLT *t,GLT_CDR_T v,GLTCMP cmp)
{
    if (!t) return 0 ;
    if (!cmp) return glrassq(t,v) ;
    for ( ; t->car || t->cdr ; t++)
	if (!cmp(t->cdr,v)) return t ;
    return 0 ;
    }

extern GLT_CDR_T glget(GLT *t,GLT_CAR_T v)
{
    GLT *tt = glassq(t,v) ;
    return tt ? tt->cdr : 0 ;
    }

extern GLT_CAR_T glrget(GLT *t,GLT_CDR_T v)
{
    GLT *tt = glrassq(t,v) ;
    return tt ? tt->car : 0 ;
    }

extern GLT_CDR_T glgetif(GLT *t,GLT_CAR_T v,GLTCMP cmp)
{
    GLT *tt = glassoc(t,v,cmp) ;
    return tt ? tt->cdr : 0 ;
    }

extern GLT_CAR_T glrgetif(GLT *t,GLT_CDR_T v,GLTCMP cmp)
{
    GLT *tt = glrassoc(t,v,cmp) ;
    return tt ? tt->car : 0 ;
    }

extern char *gltget_c(GLT *t,int v)
{
    GLT *tt = glassq(t,(GLT_CAR_T) v) ;
    if (tt) return((char *) tt->cdr) ;
    return "*unknown*" ;
    }

#ifdef __cplusplus /*Z*/
}
#endif
