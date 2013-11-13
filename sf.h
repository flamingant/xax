#ifdef __cplusplus /*Z*/
extern "C" {
#endif

#ifndef __sf_h
#define __sf_h

#define SF_O(T,F)	((int)(&((T *) 0)->F))
#define SF_P(f,base)	((void *) ((char *) base + f->offset))

#ifndef __cplusplus
typedef int bool ;
#endif

typedef enum {
    paused,
    downloading,
    seeding,
    invalid,
    } ENUM_TORRENTSTATE ;

typedef enum {
    SFT__END = -1,
    SFT_CHARP,
    SFT_INT,
    SFT_INT64,
    SFT_BOOL,
    SFT_DOUBLE,
    SFT_ENUM_TORRENTSTATE,
    } SFT ;

typedef struct {
    SFT		sft ;
    char	*name ;
    int		offset ;
    } SF ;

#endif


#ifdef __cplusplus /*Z*/
}
#endif
