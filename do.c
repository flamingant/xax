#ifdef __cplusplus /*Z*/
extern "C" {
#endif

#include	<stdio.h>
#include	<stdarg.h>
#include	<string.h>

#include	"do.h"

#define NEED_ERRORS_STDIO
#include	"errors.h"
#include	"arg.h"
#include	"npu_util.h"

GDBG gdbg ;

extern void dpf_va(char *fmt,va_list va)
{
    FILE *fh = gdbg.fh ? gdbg.fh : stderr ;
    vfprintf(fh,fmt,va) ;
    fflush(fh) ;
    }

extern void dpf(char *fmt,...)
{
    va_list	va ;
    va_start(va,fmt) ;
    dpf_va(fmt,va) ;
    }

extern void dpfn(int level,char *fmt,...)
{
    va_list	va ;
    if (level > gdbg.level) return ;
    va_start(va,fmt) ;
    dpf_va(fmt,va) ;
    }

extern void dph(char *p,int n)
{
    if (n < 0) return ;
    while (1) {
	dpf("%02x",*p++) ;
	if (--n == 0) break ;
	dpf(" ") ;
	}
    }

extern void dpfm(int mask,char *fmt,...)
{
    va_list	va ;
    if (!(mask & gdbg.mask)) return ;
    va_start(va,fmt) ;
    dpf_va(fmt,va) ;
    }

/* ================================================================ */
extern void dpf_file_set(char *file)
{
    if (gdbg.fh) fclose(gdbg.fh) ;
    if (file)
	 gdbg.fh = fopen_and_check(file,"w") ;
    else gdbg.fh = 0 ;
    }

extern void dpf_level_set(int value)
{
    gdbg.level = value ;
    }

extern void dpf_mask_set(int value)
{
    gdbg.mask = value ;
    }


#ifdef __cplusplus /*Z*/
}
#endif
