#ifdef __cplusplus /*Z*/
extern "C" {
#endif

#ifndef __log_h
#define __log_h	1

#include	<stdio.h>
#include	<stdarg.h>

#ifndef __mt_h
#include	<pu/mt.h>
#endif

typedef struct {
    char	*name ;
    int		enable ;
    char	*description ;
    } LOGSEC ;

#define LF_STDOUT		1
#define LF_STDERR		2
#define LF_OPENFAIL		4

#define LF_TIMESTAMP		8
#define LF_SEQUENCE		16

extern LOGSEC ls_generic[] ;

extern int log_init(int,char **) ;

extern void log_write_s(LOGSEC *,char *) ;
extern void log_write(LOGSEC *,char *,int) ;
extern void log_puts(LOGSEC *,char *) ;
extern void log_printf(LOGSEC *,char *fmt,...) ;

extern void log_close(void) ;
extern void log_set(FILE *f) ;
extern void log_open(char *file) ;

extern void log_vprintf(LOGSEC *ls,char *fmt,va_list va) ;

extern void log_prefix_set(int n) ;

/* continuation - no prefix is output */

extern void log_puts_c(LOGSEC *,char *) ;
extern void log_printf_c(LOGSEC *,char *fmt,...) ;
extern void log_vprintf_c(LOGSEC *ls,char *fmt,va_list va) ;

extern void log_put_escape_nl(LOGSEC *ls,char *s,int len) ;
extern void log_puts_escape_nl(LOGSEC *ls,char *s) ;
extern void log_put_to_first_nl(LOGSEC *ls,char *s,int len) ;
extern void log_puts_to_first_nl(LOGSEC *ls,char *s) ;

extern void mt_put_timestamp(MT *mt) ;

extern void log_printg_va(LOGSEC *ls,char *fmt,va_list va) ;
extern void log_printg(LOGSEC *ls,char *fmt,...) ;

#endif

#ifdef __cplusplus /*Z*/
}
#endif
