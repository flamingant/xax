#ifdef __cplusplus /*Z*/
extern "C" {
#endif

#ifndef __glt_h
#define __glt_h

#ifndef __GLT_CAR_T_DEFINED
typedef void *GLT_CAR_T ;
#endif

#ifndef __GLT_CDR_T_DEFINED
typedef void *GLT_CDR_T ;
#endif

typedef struct {
    GLT_CAR_T	car ;
    GLT_CDR_T	cdr ;
    } GLT ;

#define GLTITEM(a,b)	{(GLT_CAR_T) a,(GLT_CDR_T) b}

#define GLTENDTABLE	GLTITEM(0,0)

typedef int (*GLTCMP)(GLT_CAR_T,GLT_CDR_T) ;

extern int glt_count(GLT *t) ;

extern GLT *glassq(GLT *t,GLT_CAR_T v) ;
extern GLT *glrassq(GLT *t,GLT_CDR_T v) ;

extern GLT_CDR_T glget(GLT *,GLT_CAR_T) ;
extern GLT_CAR_T glrget(GLT *,GLT_CDR_T) ;

extern GLT *glassoc(GLT *,GLT_CAR_T,GLTCMP) ;
extern GLT *glrassoc(GLT *,GLT_CDR_T,GLTCMP) ;

extern GLT_CDR_T glgetif(GLT *,GLT_CAR_T,GLTCMP) ;
extern GLT_CAR_T glrgetif(GLT *,GLT_CDR_T,GLTCMP) ;

extern char *gltget_c(GLT *t,int v) ;

#endif

#ifdef __cplusplus /*Z*/
}
#endif
