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

GDO gdo ;

extern void dof_va(char *fmt,va_list va)
{
    FILE *fh = gdo.fh ? gdo.fh : stderr ;
    vfprintf(fh,fmt,va) ;
    fflush(fh) ;
    }

extern void dof(char *fmt,...)
{
    va_list	va ;
    va_start(va,fmt) ;
    dof_va(fmt,va) ;
    }

extern void dofn(int level,char *fmt,...)
{
    va_list	va ;
    if (level > gdo.level) return ;
    va_start(va,fmt) ;
    dof_va(fmt,va) ;
    }

extern void doph(char *p,int n)
{
    if (n < 0) return ;
    while (1) {
	dof("%02x",*p++) ;
	if (--n == 0) break ;
	dof(" ") ;
	}
    }

extern void dofm(int mask,char *fmt,...)
{
    va_list	va ;
    if (!(mask & gdo.mask)) return ;
    va_start(va,fmt) ;
    dof_va(fmt,va) ;
    }

/* ================================================================ */
extern void dof_file_set(char *file)
{
    if (gdo.fh) fclose(gdo.fh) ;
    if (file)
	 gdo.fh = fopen_and_check(file,"w") ;
    else gdo.fh = 0 ;
    }

extern void dof_level_set(int value)
{
    gdo.level = value ;
    }

extern void dof_mask_set(int value)
{
    gdo.mask = value ;
    }


#ifdef __cplusplus /*Z*/
}
#endif
