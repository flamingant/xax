#ifdef __cplusplus /*Z*/
extern "C" {
#endif

#ifndef __mallog_h
#define __mallog_h

#include	<malloc.h>

typedef struct __struct_GMC GMC ;
typedef struct __struct_GMCT GMCT ;

#ifndef GMCD_DEFINED
typedef void *GMCD ;
#endif

extern void *gmalloc(GMC *,size_t) ;
extern void *grealloc(GMC *,void *,size_t) ;
extern void *gcalloc(GMC *,size_t,size_t) ;
extern void  gfree(GMC *,void *) ;
extern char *gstrdup(GMC *,char *) ;

extern void gmalloc_close(GMC *c) ;
/* ================================================================ */
#ifdef GMC_INTERNALS

struct __struct_GMCT {
    long	(*generic)(GMC *,int,long) ;
    void *	(*malloc)(GMC *,size_t) ;
    void *	(*realloc)(GMC *,void *p,size_t) ;
    void	(*free)(GMC *,void *) ;
    } ;

#endif

struct __struct_GMC {
    GMCT	*t ;
    GMCD	d ;
    char	_d_[16] ;
    } ;

#define GMM_CLOSE	0

extern GMC gmc_std_malloc[] ;

extern long gmc_generic_null(GMC *,int,long) ;

extern void mt_malloc_init(GMC *c,int size) ;

extern void mt_malloc_close(GMC *c) ;
extern GMC *mt_malloc_open(int size) ;

#endif

#ifdef __cplusplus /*Z*/
}
#endif
