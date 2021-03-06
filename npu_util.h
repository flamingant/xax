#ifdef __cplusplus /*Z*/
extern "C" {
#endif

#ifndef __npu_util_h
#define __npu_util_h	1

#ifndef __talloc_h
#include	"talloc.h"
#endif

#include	"errors.h"

extern char *memtocstring(char *s,int len) ;

extern char *mprintf(char *fmt,...) ;

extern void freenz(void *) ;

#define dont_free(x)

#include	<pu/mt.h>

extern int mt_file_contents(MT *mt,char *name,int mode) ;

extern void mt_set_line(MT *m) ;
extern void mtstrdup_all(MT *a) ;

extern void mtstrdup_fill_pad(MT *a,int n) ;

extern void mtcpymt_all_escape_nl(MT *mtd,MT *mts) ;
extern void mtcpymt_to_first_nl(MT *mtd,MT *mts) ;

extern char *mtcaches(MT *mt,char *s) ;

extern void hsg_init(int argc,char **argv) ;

#ifdef __regex_h
extern char *re_compile_pattern_s(REGEX *r,char *s) ;
#endif

extern void strcpy_lc(char *d,char *s) ;

#ifdef va_start
extern int mtprintg_va(MT *mt,char *fmt,va_list va) ;
#endif

extern int mtprintg(MT *mt,char *fmt,...) ;

extern char const *tfstring(int) ;

extern void qlistbuild(MT *mt,int argc,char **argv) ;
extern void vqlistbuild(MT *mt,...) ;

#if defined(__linux__) || defined(__unix__)
#define stricmp strcasecmp
#endif

#endif

#ifdef __cplusplus /*Z*/
}
#endif
