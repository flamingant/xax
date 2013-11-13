#ifdef __cplusplus /*Z*/
extern "C" {
#endif

#include	<stdio.h>
#include	<stdarg.h>
#include	<stdlib.h>
#include	<malloc.h>
#include	<unistd.h>
#include	<ctype.h>
#include	<string.h>
#include	<fcntl.h>
#include	<sys/stat.h>

#include	<pu/regex.h>
#include	<pu/mt.h>

#include	"common.h"

extern void strcpy_lc(char *d,char *s)
{
    while (*s) *d++ = tolower((int) *s++) ;
    *d = 0 ;
    }

extern char *mprintf_va(char *fmt,va_list va)
{
    char	*s = (char *) malloc(4096) ;
    int		n ;
    n = vsnprintf(s,4096,fmt,va) ;
    return((char *) realloc(s,n+1)) ;
    }

extern char *mprintf(char *fmt,...)
{
    va_list	va ;
    va_start(va,fmt) ;
    return(mprintf_va(fmt,va)) ;
    }

extern void freenz(void *x)
{
    if (x) free(x) ;
    }

extern char *memtocstring(char *s,int len)
{
    char	*o = (char *) malloc(len+1) ;
    memcpy(o,s,len) ;
    o[len] = 0 ;
    return o ;
    }

/* ================================================================ */
extern MT *mtauto(int size)
{
    MT *r = (MT *) malloc(sizeof(MT)) ;
    mtmalloc(r,size) ;
    return r ;
    }

extern void mtauto_free(MT *mt)
{
    free(mt->s) ;
    free(mt) ;
    }

/* ================================================================ */
/* cache a zero-terminated string in MT and return pointer to cached copy */

extern char *mtcaches(MT *mt,char *s)
{
    char	*r = mt->c ;
    mtputs(mt,s) ;
    mtputc(mt,0) ;
    return(r) ;
    }

/* read file into an empty MT */

extern int mt_file_contents(MT *mt,char *name,int mode)
{
    struct stat statbuf[1] ;
    int		fd ;
    int		cb ;
    if ((fd = open(name,mode)) == -1) goto fail ;
    if (fstat(fd,statbuf) < 0) goto fail ;
    mtmalloc(mt,statbuf->st_size+1) ;		/* add a zero at end - can't hurt */
    cb = read(fd,mt->s,statbuf->st_size) ;
    MTADVANCE(mt,cb) ;
    *mt->c = 0 ;
    close(fd) ;
    return(0) ;
fail:
    return(-1) ;
    }
   
/* make MT describe only only the data to the end of the line */

extern void mt_set_line(MT *m)
{
    char	*s = m->c ;
    while (s < m->e && *s != '\r' && *s != '\n') s++ ;
    m->s = m->c ;
    m->e = s ;
    }

/* replace MT description with malloc duplicated copy up to fill pointer */

extern void mtstrdup_fill_pad(MT *a,int n)
{
    int cb = MTFillSize(a) ;
    char *s = a->s ;
    if (s) {
	mtmalloc(a,cb+n) ;
	memcpy(a->s,s,cb) ;
	MTADVANCE(a,cb) ;
    }
    }

/* replace MT description with malloc duplicated copy up to end */

extern void mtstrdup_all(MT *a)
{
    int cb = MTAllSize(a) ;
    char *s = a->s ;
    if (s) {
	mtmalloc(a,cb+1) ;
	memcpy(a->s,s,cb) ;
	a->s[cb] = 0 ;
	MTADVANCE(a,cb) ;
    }
    }

/* like strspn for MT, but with limit offset */

extern int mtspn_n(MT *m,char *s,int max)
{
    char *p = m->c ;
    char *e = m->e ;
    int n ;
    if (max < e-p) e = p+max ;
    while (p < e && strchr(s,*p)) p++ ;
    n = p - m->c ;
    m->c = p ;
    return n ;
    }

/* like strspn for MT */

extern int mtspn(MT *m,char *s)
{
    char *p = m->c ;
    char *e = m->e ;
    int n ;
    while (p < e && strchr(s,*p)) p++ ;
    n = p - m->c ;
    m->c = p ;
    return n ;
    }

extern void mt_dump_f(MT *mt,FILE *f)
{
    char	*s ;
    for (s = mt->s ; s < mt->c ; s++) fputc(*s,f) ;
    fputs("\n",f) ;
    fflush(f) ;
    }

/* copy entire source mt into a destination mt */

extern void mtcpymt_all(MT *md,MT *ms)
{
    mtput(md,ms->s,MTAllSize(ms)) ;
    }

/* copy source mt from start to fill into a destination mt */

extern void mtcpymt_prefill(MT *md,MT *ms)
{
    mtput(md,ms->s,MTFillSize(ms)) ;
    }

/* copy source mt from fill to end into a destination mt */

extern void mtcpymt_postfill(MT *md,MT *ms)
{
    mtput(md,ms->c,MTAllSize(ms) - MTFillSize(ms)) ;
    }

/* ================================================================ */
extern void mtcpymt_all_escape_nl(MT *mtd,MT *mts)
{
    char *s = mts->s ;
    char *e = mts->e ;
    char *dc = mtd->c ;
    char *de = mtd->e ;
    while (s < e) {
	if (dc >= de) break ;
	if (*s == '\r') {
	    *dc++ = '\\' ;
	    if (dc >= de) break ;
	    *dc++ = 'r' ;
	    }
	else if (*s == '\n') {
	    *dc++ = '\\' ;
	    if (dc >= de) break ;
	    *dc++ = 'n' ;
	    }
	else {
	    *dc++ = *s ;
	    }
	s++ ;
	}
    if (dc < de) *dc = 0 ;
    }

extern void mtcpymt_to_first_nl(MT *mtd,MT *mts)
{
    char *s = mts->s ;
    char *e = mts->e ;
    char *dc = mtd->c ;
    char *de = mtd->e ;
    while (s < e) {
	if (dc >= de) break ;
	if (*s == '\n') break ;
	else {
	    *dc++ = *s ;
	    }
	s++ ;
	}
    if (dc < de) *dc = 0 ;
    }

/* ================================================================ */
extern char *re_compile_pattern_s(REGEX *r,char *s)
{
    return re_compile_pattern(r,s,strlen(s)) ;
    }

/* ================================================================ */
#include	<pu/printg.h>

extern int mtprintg_va(MT *mt,char *fmt,va_list va)
{
    return sprintg_va(mt->c,MTGapSize(mt),fmt,va) ;
    }

extern int mtprintg(MT *mt,char *fmt,...)
{
    va_list	va ;
    va_start(va,fmt) ;
    return mtprintg_va(mt,fmt,va) ;
    }

/* ================================================================ */
extern char const *tfstring(int i)
{
    return i ? "true" : "false" ;
    }

extern void qlistbuild(MT *mt,int argc,char **argv)
{
    while (argc-- > 0) {
	mtprintf(mt,"\"%s\"",*argv++) ;
	if (argc > 0) mtputc(mt,',') ;
	}
    }

extern void vqlistbuild_va(MT *mt,va_list va)
{
    char *arg ;
    if (!(arg = va_arg(va,char *))) return ;
    while (1) {
	mtprintf(mt,"\"%s\",",arg) ;
	if (!(arg = va_arg(va,char *))) {
	    *(--mt->c) = 0 ;
	    return ;
	}
    }
    }

extern void vqlistbuild(MT *mt,...)
{
    va_list	va ;
    va_start(va,mt) ;
    vqlistbuild_va(mt,va) ;
    }

/* ================================================================ */
#include	"arg.h"

GOPT gopt ;

/* ~# use arg ; #~ */

/* ~~argset(help => "General options")~~ */

/* ~~arg(help => "Add torrents in paused state")~~ */
static int argf__torrent_add_paused(char *name,char *value,void *a0)
{
    gopt.add_paused = strtol(value,0,10) ;
    return(ASF_ARGACCEPTED) ;
}

#include	".gen/common.c"

#ifdef __cplusplus /*Z*/
}
#endif
