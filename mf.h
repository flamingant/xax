#ifndef __mf_h
#define __mf_h

#ifndef __type_h
#include	<pu/type.h>
#endif

typedef union {
    i64		i64 ;
    struct {
	u32	l ;
	i32	h ;
	} u32 ;
    } MFPU ;

typedef i64 MFO ;

typedef struct struct_MFM MFM ;

typedef struct struct_MF MF ;

typedef u32 (*MFFUN)(MF *,int,u32) ;
typedef u32 (*MFREADFUN)(MF *,u8 *,u32) ;
typedef u32 (*MFWRITEFUN)(MF *,u8 *,u32) ;

struct struct_MFM {
    char	*name ;
    u32		(*mf)(MFM *,int,u32) ;
    MFFUN	f ;
    MFREADFUN	read ;
    MFWRITEFUN	write ;
    int		(*_getc)(MF *) ;
    MFO		(*seek)(MF *,MFO,int) ;
    MFO		(*tell)(MF *) ;
    } ;

struct struct_MF {
    MFM		*m ;
    /* remainder is opaque */
    } ;

typedef struct struct_MFS MFS ;

struct struct_MFS {
    MF		*f ;
    MFO		start ;
    MFO		end ;
    } ;

#define MFSLENGTH(x)	(((x)->end - (x)->start) + 1)

typedef struct {
    MFO		start ;
    MFO		end ;
    } MFSN ;

#define MFMM_OPEN	0
#define MFMM_CLOSE	1

#define MFMM_GETEOFPOS	2
#define MFMM_GETCACHE	3

#define MFMM_REFINC	4
#define MFMM_REFDEC	5

#define MFMM_DESCRIBE	6

/* common seek flags */

#define MFF_SEEKUNDER	1
#define MFF_SEEKOVER	2

extern void *mf_alloc(int size) ;
extern void  mf_free(void *) ;

/* ================================================================ */
extern MFM null_mfm[] ;

extern u32 null_mf_mf(MFM *m,int i,u32 a0) ;
extern u32 null_mf_f(MF *f,int i,u32 a0) ;

extern MF *null_mf_open(void) ;
/* ================================================================ */
extern MFM mem_mfm[] ;
extern MF *mem_mf_open(u8 *,int) ;
/* ================================================================ */
typedef struct {
    MFM		*m ;
    FILE	*f ;
    } STD_MF ;

extern MFM std_mfm[] ;
extern MF *std_mf_open(FILE *) ;
/* ================================================================ */
extern MFM file_mfm[] ;
extern MF *file_mf_open(char *) ;
/* ================================================================ */
extern MFM disk_mfm[] ;
extern MF *disk_mf_open(char *) ;
/* ================================================================ */

extern void mf_close(MF *) ;

extern u32 mfseekread(MF *f,MFO o,u8 *p,u32 n) ;
extern u32 mfseekwrite(MF *f,MFO o,u8 *p,u32 n) ;

extern u32 mfgetc(MF *f) ;
extern u32 mfread(MF *f,u8 *p,u32 n) ;
extern u32 mfwrite(MF *f,u8 *p,u32 n) ;
extern MFO mfseek(MF *f,MFO o,int mode) ;

extern void mfeofpos(MF *f,MFO *a) ;

extern MFO mftell(MF *f) ;
extern MFO mftelleof(MF *f) ;

extern void mf_ref_increment(MF *f) ;
extern void mf_ref_decrement(MF *f) ;

#ifndef FILE_BEGIN

#define FILE_BEGIN	0
#define FILE_CURRENT	1
#define FILE_END	2

#endif

#endif
