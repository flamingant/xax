#ifdef __cplusplus /*Z*/
extern "C" {
#endif

#include	<stdlib.h>
#include	<stdio.h>

#include	"errors.h"
#include	"sql.h"

extern void sql_errorshow(sqlite3 *db,char *f)
{
    errorshow(f, sqlite3_errmsg(db));
    }

extern void sql_errorcheck(sqlite3 *db,char *f,int rc)
{
    if (!f) f = "error: %s" ;
    if (rc != 0) sql_errorshow(db,f) ;
    }

extern void sql_errorfatal(sqlite3 *db,char *f)
{
    errorshow(f, sqlite3_errmsg(db));
    exit(1) ;
    }

extern SQLITE_API int sqlite3_open_v2_or(
  int what,
  const char *filename,   /* Database filename (UTF-8) */
  sqlite3 **ppDb,         /* OUT: SQLite db handle */
  int flags,              /* Flags */
  const char *zVfs        /* Name of VFS module to use */
)
{
    int r = sqlite3_open_v2(filename,ppDb,flags,zVfs) ;
    if (r)
	errorshow_and(what,
		      "sqlite3_open_v2 couldn't open %s - %s\n",
		      filename,
		      sqlite3_errmsg(*ppDb)) ;
    return(r) ;
    }

extern SQLITE_API int sqlite3_exec_or(
  int what,
  sqlite3 *db,                               /* An open database */
  const char *sql,                           /* SQL to be evaluated */
  int (*callback)(void*,int,char**,char**),  /* Callback function */
  void *arg,                                 /* 1st argument to callback */
  char **errmsg                              /* Error msg written here */
)
{
    int r = sqlite3_exec(db,sql,callback,arg,errmsg) ;
    if (r)
	errorshow_and(what,
		      "sqlite3_exec failed for \"%s\" - %s\n",
		      sql,
		      sqlite3_errmsg(db)) ;
    return(r) ;
    }

extern SQLITE_API int sqlite3_prepare_v2_or(
  int what,		  /* or else what */
  sqlite3 *db,            /* Database handle */
  const char *zSql,       /* SQL statement, UTF-8 encoded */
  int nByte,              /* Maximum length of zSql in bytes. */
  sqlite3_stmt **ppStmt,  /* OUT: Statement handle */
  const char **pzTail     /* OUT: Pointer to unused portion of zSql */
)
{
    int r = sqlite3_prepare_v2(db,zSql,nByte,ppStmt,pzTail) ;
    if (r)
	errorshow_and(what,
		      "sqlite3_prepare_v2 failed for \"%s\" - %s\n",
		      zSql,
		      sqlite3_errmsg(db)) ;
    return(r) ;
    }

extern SQLITE_API int sqlite3_step_or(
  int what,
  sqlite3 *db,
  sqlite3_stmt *stmt)
{
    int r = sqlite3_step(stmt) ;
    if (r != SQLITE_DONE && r != SQLITE_ROW)
	errorshow_and(what,
		      "sqlite3_step failed - %s\n",
		      sqlite3_errmsg(db)) ;
    return(r) ;
    }

/* ================================================================ */
#include	<stdarg.h>
#include	<malloc.h>
#include	<pu/mt.h>

extern int sqlexecf_va(sqlite3 *db,char *fmt,va_list va)
{
    MT	mt[1] ;
    MTALLOCA(mt,4096) ;
    *(--mt->e) = 0 ;		/* guarantees zero termination */
    mtvprintf(mt,fmt,va) ;
    return(sqlite3_exec(db,mt->s,0,0,0)) ;
    }

extern int sqlexecf(sqlite3 *db,char *fmt,...)
{
    va_list	va ;
    va_start(va,fmt) ;
    return(sqlexecf_va(db,fmt,va)) ;
    }

extern int sqlexecf_or_va(int what,sqlite3 *db,char *fmt,va_list va)
{
    MT	mt[1] ;
    MTALLOCA(mt,4096) ;
    *(--mt->e) = 0 ;		/* guarantees zero termination */
    mtvprintf(mt,fmt,va) ;
    return(sqlite3_exec_or(what,db,mt->s,0,0,0)) ;
    }

extern int sqlexecf_or(int what,sqlite3 *db,char *fmt,...)
{
    va_list	va ;
    va_start(va,fmt) ;
    return(sqlexecf_or_va(what,db,fmt,va)) ;
    }

extern int sqlpreparef_or_va(int what,sqlite3 *db,sqlite3_stmt **ppStmt,char *fmt,va_list va)
{
    MT	mt[1] ;
    MTALLOCA(mt,4096) ;
    *(--mt->e) = 0 ;		/* guarantees zero termination */
    mtvprintf(mt,fmt,va) ;
    return(sqlite3_prepare_v2_or(what,db,mt->s,-1,ppStmt,0)) ;
    }

extern int sqlpreparef_or(int what,sqlite3 *db,sqlite3_stmt **ppStmt,char *fmt,...)
{
    va_list	va ;
    va_start(va,fmt) ;
    return(sqlpreparef_or_va(what,db,ppStmt,fmt,va)) ;
    }

/* ================================================================ */
extern int sql_single_row_int(sqlite3 *db,char *sql,int *v)
{
    sqlite3_stmt	*stmt ;
    int rc ;
    if ((rc = sqlite3_prepare_v2(db,sql,-1,&stmt,NULL)) != SQLITE_OK)
	return(rc) ;
    if ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
	*v = sqlite3_column_int(stmt,0) ;
	}
    sqlite3_finalize(stmt) ;
    return(rc) ;
    }
    
    
/* ================================================================ */
extern int sqlite3_bind_int_string(sqlite3_stmt *st,int i,char *s)
{
    int		n,r ;
    n = strtol(s,0,10) ;
    r = sqlite3_bind_int(st,i,n) ;
    return r ;
    }

extern int sqlite3_bind_cstring(sqlite3_stmt *st,int i,char *s)
{
    int		r ;
    if (s) 
	 r = sqlite3_bind_text(st,i,s,-1,SQLITE_TRANSIENT) ;
    else r = 0 ;
    return r ;
    }


#ifdef __cplusplus /*Z*/
}
#endif
