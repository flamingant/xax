/*(cg-start)*/

#include	<malloc.h>
#include	<stdio.h>
#include	<string.h>

#include	<pu/util.h>
#include	<pu/mt.h>

#include	"mf.h"

#define file_mf_open__mgrab(x)	strdup(x)
#define file_mf_open__mdrop(x)	free(x)

/* the idea behind this mechanism is to provide a transparent
   interface to "virtual memory" devices which can make file (or disk)
   contents accessible as if the whole file is resident in memory.
   The purpose is to allow access to very large (i.e > 2^32 bytes) files.
   */

/* ================================================================ */
extern void *mf_alloc(int size)
{
    return(calloc(1,size)) ;
    }

extern void mf_free(void *mf)
{
    free(mf) ;
    }

/* ================================================================ */
extern MFO mftell(MF *f)
{
    return(f->m->tell(f)) ;
    }

extern MFO mfseek(MF *f,MFO o,int mode)
{
    return(f->m->seek(f,o,mode)) ;
    }

extern u32 mfread(MF *f,u8 *p,u32 n)
{
    return(f->m->read(f,p,n)) ;
    }

extern u32 mfwrite(MF *f,u8 *p,u32 n)
{
    return(f->m->write(f,p,n)) ;
    }

extern u32 mfgetc(MF *f)
{
    return(f->m->_getc(f)) ;
    }

/* ================================================================ */
extern void mfeofpos(MF *f,MFO *a)
{
    f->m->f(f,MFMM_GETEOFPOS,(u32) a) ;
    }

extern void mf_ref_increment(MF *f)
{
    f->m->f(f,MFMM_REFINC,0) ;
    }

extern void mf_ref_decrement(MF *f)
{
    f->m->f(f,MFMM_REFDEC,0) ;
    }

extern void mf_close(MF *mf)
{
    mf->m->f(mf,MFMM_CLOSE,0) ;
    }

extern MFO mftelleof(MF *f)
{
    MFO o ;
    f->m->f(f,MFMM_GETEOFPOS,(u32) &o) ;
    return(o) ;
    }

extern u32 mfseekread(MF *f,MFO o,u8 *p,u32 n)
{
    f->m->seek(f,o,FILE_BEGIN) ;
    return(f->m->read(f,p,n)) ;
    }

extern u32 mfseekwrite(MF *f,MFO o,u8 *p,u32 n)
{
    f->m->seek(f,o,FILE_BEGIN) ;
    return(f->m->write(f,p,n)) ;
    }

/* ================================================================ */
/* NULL	no operations						    */
/* ================================================================ */
extern u32 null_mf_mf(MFM *m,int i,u32 a0)
{
    switch(i) {
    default:
	return(0) ;
	}
    }

extern u32 null_mf_f(MF *f,int i,u32 a0)
{
    switch(i) {
    case MFMM_CLOSE:
	return(0) ;
    case MFMM_GETCACHE:
	return(0) ;
    case MFMM_GETEOFPOS:
	*((MFO *) a0) = -1 ;
	return(0) ;
    case MFMM_DESCRIBE:
	mtprintf((MT *) a0,"%s %08X",f->m->name,f) ;
	return(0) ;
    default:
	return(0) ;
	}
    }

extern int null_mf_getc(MF *f)
{
    return(-1) ;
    }

MFM null_mfm[] = {
    "null",
    null_mf_mf,
    (MFFUN) null_mf_f,
    (MFREADFUN) return_0,
    (MFWRITEFUN) return_0,
    (int (*)(MF *)) null_mf_getc,
    (MFO (*)(MF *,MFO,int)) return_0,
    (MFO (*)(MF *)) return_0,
} ;

extern MF *null_mf_open(void)
{
    MF *f = mf_alloc(sizeof(MF)) ;
    f->m = null_mfm ;
    return((MF *) f) ;
    }

/* ================================================================ */
/* MEM	wrapper memory buffer					    */
/* ================================================================ */
typedef struct {
    MFM		*m ;
    MFO		o ;
    int		flags ;
    u8		*s ;
    MFO		n ;
    } MEM_MF ;

static u32 mem_mf_mf(MFM *m,int i,u32 a0)
{
    switch(i) {
    default:
	return(null_mf_mf(m,i,a0)) ;
	}
    }

static u32 mem_mf_f(MEM_MF *f,int i,u32 a0)
{
    switch(i) {
    case MFMM_CLOSE:
	mf_free(f) ;
	return(0) ;
    case MFMM_GETCACHE:
	return((u32) f->s) ;
    case MFMM_OPEN:
	goto def ;
    case MFMM_GETEOFPOS:
	*((MFO *) a0) = f->n ;
	return(0) ;
    case MFMM_DESCRIBE:
	mtprintf((MT *) a0,"%s %08X",f->m->name,f) ;
	return(0) ;
    default:
	goto def ;
	}
def:
    return(null_mf_f((MF *) f,i,a0)) ;
    }

#define NOCHECK(stuff)

static MFO mem_mf_seek(MEM_MF *f,MFO o,int mode)
{
    i64 omax = f->n ;
    switch(mode) {
    case FILE_BEGIN:
	f->o = o ;
	break ;
    case FILE_CURRENT:
	f->o += o ;
	break ;
    case FILE_END:
	f->o = omax - o ;
	break ;
    }
    f->flags &= ~(MFF_SEEKUNDER | MFF_SEEKOVER) ;
    if (f->o > omax) {
	f->flags |= MFF_SEEKOVER ;
	o = omax ;
        }
    if (f->o < 0) {
	f->flags |= MFF_SEEKUNDER ;
	f->o = 0 ;
        }
    return(f->o) ;
    }

static u32 mem_mf_read(MEM_MF *f,u8 *p,u32 n)
{
    int na = f->n - f->o ;
    if (na <= 0) return(0) ;
    if (n > na) n = na ;
    memcpy(p,f->s + f->o,n) ;
    f->o += n ;
    return(n) ;
    }

static u32 mem_mf_write(MEM_MF *f,u8 *p,u32 n)
{
    int na = f->n - f->o ;
    if (na <= 0) return(0) ;
    if (n > na) n = na ;
    memcpy(f->s + f->o,p,n) ;
    f->o += n ;
    return(n) ;
    }

static int mem_mf_getc(MEM_MF *f)
{
    if (f->o < f->n) return(f->s[f->o++]) ;
    return(-1) ;
    }

static MFO mem_mf_tell(MEM_MF *f)
{
    return(f->o) ;
    }

MFM mem_mfm[] = {
    "mem",
    mem_mf_mf,
    (MFFUN) mem_mf_f,
    (MFREADFUN) mem_mf_read,
    (MFWRITEFUN) mem_mf_write,
    (int (*)(MF *)) mem_mf_getc,
    (MFO (*)(MF *,MFO,int)) mem_mf_seek,
    (MFO (*)(MF *)) mem_mf_tell,
} ;

extern MF *mem_mf_open(u8 *s,int n)
{
    MEM_MF *f = mf_alloc(sizeof(MEM_MF)) ;
    f->m = mem_mfm ;
    f->s = s ;
    f->n = n ;
    return((MF *) f) ;
    }

/* ================================================================ */
/* STD	wrapper around stdio FILE stream			    */
/* ================================================================ */
typedef struct {
    MFM		*m ;
    FILE	*f ;
    } STD_MF ;

static u32 std_mf_mf(MFM *m,int i,u32 a0)
{
    switch(i) {
    default:
	return(null_mf_mf(m,i,a0)) ;
	}
    }

static u32 std_mf_f(STD_MF *f,int i,u32 a0)
{
    switch(i) {
    case MFMM_CLOSE:
	/* to free or not to free */
	return(0) ;
    case MFMM_GETCACHE:
    case MFMM_OPEN:
	goto def ;
    case MFMM_GETEOFPOS: {
	MFO c ;
	c = ftell(f->f) ;
	fseek(f->f,0,SEEK_END) ;
	*((MFO *) a0) = ftell(f->f)  ;
	fseek(f->f,c,SEEK_SET) ;
	return(0) ;
	}
    case MFMM_DESCRIBE:
	return(0) ;
    default:
	goto def ;
	}
def:
    return(null_mf_f((MF *) f,i,a0)) ;
    }

static u32 std_mf_read(STD_MF *f,u8 *p,u32 n)
{
    return fread(p,1,n,f->f) ;
    }

static u32 std_mf_write(STD_MF *f,u8 *p,u32 n)
{
    return fwrite(p,1,n,f->f) ;
    }

static int std_mf_getc(STD_MF *f)
{
    return(fgetc(f->f)) ;
    }

static MFO std_mf_seek(STD_MF *f,MFO o,int mode)
{
    return(fseek(f->f,o,mode)) ;
    }

static MFO std_mf_tell(STD_MF *f)
{
    return(ftell(f->f)) ;
    }

MFM std_mfm[] = {
    "std",
    std_mf_mf,
    (MFFUN) std_mf_f,
    (MFREADFUN) std_mf_read,
    (MFWRITEFUN) std_mf_write,
    (int (*)(MF *)) std_mf_getc,
    (MFO (*)(MF *,MFO,int)) std_mf_seek,
    (MFO (*)(MF *)) std_mf_tell,
} ;

extern MF *std_mf_open(FILE *f)
{
    STD_MF *mf = mf_alloc(sizeof(STD_MF)) ;
    mf->m = std_mfm ;
    mf->f = f ;
    return((MF *) mf) ;
    }

#ifndef __linux__
/* ================================================================ */
/* WFILE	wrapper windows files				    */
/* ================================================================ */

#include	"windows.h"

#define CACHESECTORS	8
#define SECTORSIZE	512

typedef struct {
    MFM		*m ;
    MFO		o ;
    int		flags ;

    HANDLE	hf ;
    char	*name ;

    struct {
	MFO	v ;
	int	valid ;
	} size ;

    int		mruslot ;
    int		nextslot ;
    struct {
	int	age ;
	MFO	o ;
	} index[CACHESECTORS] ;
    u8		sectors[CACHESECTORS][SECTORSIZE] ;

    DWORD	accessMode ;
    DWORD	shareMode ;

    } FILE_MF ;

extern void GetFileLength(FILE_MF *f,MFO *p)
{
    MFPU cp,ep ;
    cp.u32.h = 0 ;
    ep.u32.h = 0 ;
    cp.u32.l = SetFilePointer(f->hf,0,&cp.u32.h,FILE_CURRENT) ;
    ep.u32.l = SetFilePointer(f->hf,0,&ep.u32.h,FILE_END) ;
    cp.u32.l = SetFilePointer(f->hf,cp.u32.l,&cp.u32.h,FILE_BEGIN) ;
    *p = ep.i64 ;
    }

static u32 file_mf_mf(MFM *m,int i,u32 a0)
{
    switch(i) {
    default:
	return(null_mf_mf(m,i,a0)) ;
	}
    }

static u32 file_mf_f(FILE_MF *f,int i,u32 a0)
{
    switch(i) {
    case MFMM_GETCACHE:
	return(0) ;
    case MFMM_OPEN:
	goto def ;
    case MFMM_GETEOFPOS: {
	GetFileLength(f,(MFO *) a0) ;
	return(0) ;
	}
    case MFMM_CLOSE:
	CloseHandle(f->hf) ;
	file_mf_open__mdrop(f->name) ;
	return(0) ;
    case MFMM_DESCRIBE:
	mtprintf((MT *) a0,"%s %08X name: %s mode:%04x",f->m->name,f,f->name,f->accessMode) ;
	return(0) ;
    default:
	goto def ;
	}
def:
    return(null_mf_f((MF *) f,i,a0)) ;
    }

static u32 file_mf_read(FILE_MF *f,u8 *p,u32 n)
{
    u8 *ps = p ;
    int i,e ;
    u32 cbr ;
    MFO o = f->o ;
    int sector_offset = o % SECTORSIZE ;
    int sector_partial = SECTORSIZE - sector_offset ;
    MFO sector_start = o - sector_offset ;
    MFO nmax = (f->size.v - o) ;
    if (nmax < 0) {
	return(0) ;
	}
    if (n > nmax) n = nmax ;
    while (n > 0) {
	i = f->mruslot ;
	for ( ; ; ) {
	    if (sector_start == f->index[i].o) {
		int ns = sector_partial ;
		if (ns > n) ns = n ;
		memcpy(p,f->sectors[i] + sector_offset,ns) ;
		n -= ns ;
		p += ns ;
		sector_start += SECTORSIZE ;
		sector_partial = SECTORSIZE ;
		sector_offset = 0 ;
		goto gotchunk ;
		}
	    i = (i + 1) % CACHESECTORS ;
	    if (i == f->mruslot) break ;
	    }
	if ((e = SetFilePointer(f->hf,sector_start,0,FILE_BEGIN)) == INVALID_SET_FILE_POINTER) {
	    e = GetLastError() ;
	}
	i = f->nextslot ;
	f->nextslot = (f->nextslot + 1) % CACHESECTORS ;
	f->index[i].o = sector_start ;
	ReadFile(f->hf,f->sectors[i],SECTORSIZE,&cbr,NULL) ;
gotchunk:
	f->mruslot = i ;
	continue ;
	}
    n = (p - ps) ;
    f->o += n ;
    return(n) ;
    }

static u32 file_mf_write(FILE_MF *f,u8 *p,int n)
{
    return(0) ;
    }

/* note we do not use SetFilePointer here. we just store the desired address */

static int file_mf_getc(FILE_MF *f)
{
    u8 c ;
    int n ;
    if ((n = file_mf_read(f,&c,1)) == 0) return(EOF) ;
    return(c) ;
    }

static MFO file_mf_seek(FILE_MF *f,MFO o,int mode)
{
    i64 omax ;
    if (!f->size.valid) {
	GetFileLength(f,&f->size.v) ;
	f->size.valid = 1 ;
	}
    omax = f->size.v ;
    switch(mode) {
    case FILE_BEGIN:
	f->o = o ;
	break ;
    case FILE_CURRENT:
	f->o += o ;
	break ;
    case FILE_END:
	f->o = omax - o ;
	break ;
    }
    f->flags &= ~(MFF_SEEKUNDER | MFF_SEEKOVER) ;
    if (f->o > omax) {
	f->flags |= MFF_SEEKOVER ;
	o = omax ;
        }
    if (f->o < 0) {
	f->flags |= MFF_SEEKUNDER ;
	f->o = 0 ;
        }
    return(f->o) ;
    }

static MFO file_mf_tell(FILE_MF *f)
{
    return(f->o) ;
    }

MFM file_mfm[] = {
    "file",
    file_mf_mf,
    (MFFUN) file_mf_f,
    (MFREADFUN) file_mf_read,
    (MFWRITEFUN) file_mf_write,
    (int (*)(MF *)) file_mf_getc,
    (MFO (*)(MF *,MFO,int)) file_mf_seek,
    (MFO (*)(MF *)) file_mf_tell,
} ;

static int file_mf_open_try(FILE_MF *f,DWORD accessMode,DWORD shareMode)
{
    f->hf = CreateFile(f->name,
		       accessMode,
		       shareMode,
		       NULL,
		       OPEN_EXISTING,
		       0,
		       NULL) ;
    if (f->hf == INVALID_HANDLE_VALUE) return(GetLastError()) ;
    f->accessMode = accessMode ;
    f->shareMode = shareMode ;
    return(0) ;
    }

extern MF *file_mf_open(char *name)
{
    int e ;
    FILE_MF *f = mf_alloc(sizeof(FILE_MF)) ;
    f->name = file_mf_open__mgrab(name) ;
    f->m = file_mfm ;

    do {
	if (!(e = file_mf_open_try(f,
				   GENERIC_READ | GENERIC_WRITE,
				   FILE_SHARE_READ | FILE_SHARE_WRITE)))
	    break ;
	if (!(e = file_mf_open_try(f,
				   GENERIC_READ,
				   FILE_SHARE_READ)))
	    break ;

	file_mf_open__mdrop(f->name) ;
	mf_free(f) ;
	return(0) ;
	} while (0) ;
{
    int i ;
    for (i = 0 ; i < CACHESECTORS ; i++) {
	f->index[i].o = -1 ;
    }
    }
    GetFileLength(f,&f->size.v) ;
    f->size.valid = 1 ;
    return((MF *) f) ;
    }

/* ================================================================ */
/* DISK		windows disk device				    */
/* ================================================================ */
#include	<winioctl.h>

extern void GetDiskLength_2000(FILE_MF *f,MFO *p)
{
    ULARGE_INTEGER a,b,c ;
    char name[1024] ;
    snprintf(name,sizeof(name),"%s\\",f->name) ;
    GetDiskFreeSpaceEx(name,&a,&b,&c) ;
    *p = *((MFO *) &b) ;
    }

extern void GetDiskLength_2003XP(FILE_MF *f,MFO *p)
{
    u32 cbr = 0 ;
    DISK_GEOMETRY_EX dgi[1] ;
    GET_LENGTH_INFORMATION info[1] ;
    int e ;
    e = DeviceIoControl(f->hf,IOCTL_DISK_GET_DRIVE_GEOMETRY_EX,
			NULL,0,
			(void *) dgi,sizeof(dgi),
			&cbr,0) ;
    if (!e) e = GetLastError() ;
    e = DeviceIoControl(f->hf,IOCTL_DISK_GET_LENGTH_INFO,
			NULL,0,
			(void *) info,sizeof(info),
			&cbr,0) ;
    if (!e) e = GetLastError() ;
    }

/* ================================================================ */
static u32 disk_mf_f(FILE_MF *f,int i,u32 a0)
{
    switch(i) {
    case MFMM_GETEOFPOS: {
	if (!f->size.valid) {
	    GetDiskLength_2000(f,&f->size.v) ;
	    f->size.valid = 1 ;
	}
	*((MFO *) a0) = f->size.v ;
	return(0) ;
	}
    case MFMM_DESCRIBE:
	mtprintf((MT *) a0,"%s %08X",f->m->name,f) ;
	return(0) ;
    default:
	goto def ;
	}
def:
    return(file_mf_f(f,i,a0)) ;
    }

MFM disk_mfm[] = {
    "disk",
    file_mf_mf,
    (MFFUN) disk_mf_f,
    (MFREADFUN) file_mf_read,
    (MFWRITEFUN) file_mf_write,
    (int (*)(MF *)) file_mf_getc,
    (MFO (*)(MF *,MFO,int)) file_mf_seek,
    (MFO (*)(MF *)) file_mf_tell,
} ;

extern MF *disk_mf_open(char *name)
{
    FILE_MF *f = mf_alloc(sizeof(FILE_MF)) ;
    f->name = file_mf_open__mgrab(name) ;
    f->m = disk_mfm ;
    f->hf = CreateFile(name,
		       GENERIC_READ | GENERIC_WRITE,
		       FILE_SHARE_READ | FILE_SHARE_WRITE,
		       NULL,
		       OPEN_EXISTING,
		       0,
		       NULL) ;
{
    int i ;
    for (i = 0 ; i < CACHESECTORS ; i++) {
	f->index[i].o = -1 ;
    }
    }
    return((MF *) f) ;
    }

#endif
