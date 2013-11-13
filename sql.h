#ifdef __cplusplus /*Z*/
extern "C" {
#endif

#ifndef __sql_h
#define __sql_h

#include	<sqlite3.h>

extern void sql_errorshow(sqlite3 *db,char *f) ;
extern void sql_errorfatal(sqlite3 *db,char *f) ;
extern void sql_errorcheck(sqlite3 *db,char *f,int rc) ;

extern SQLITE_API int sqlite3_open_v2_or(
  int what,		  /* or else what */
  const char *filename,   /* Database filename (UTF-8) */
  sqlite3 **ppDb,         /* OUT: SQLite db handle */
  int flags,              /* Flags */
  const char *zVfs        /* Name of VFS module to use */
) ;

extern SQLITE_API int sqlite3_exec_or(
  int what,
  sqlite3 *db,                               /* An open database */
  const char *sql,                           /* SQL to be evaluated */
  int (*callback)(void*,int,char**,char**),  /* Callback function */
  void *arg,                                 /* 1st argument to callback */
  char **errmsg                              /* Error msg written here */
) ;

extern SQLITE_API int sqlite3_prepare_v2_or(
  int what,		  /* or else what */
  sqlite3 *db,            /* Database handle */
  const char *zSql,       /* SQL statement, UTF-8 encoded */
  int nByte,              /* Maximum length of zSql in bytes. */
  sqlite3_stmt **ppStmt,  /* OUT: Statement handle */
  const char **pzTail     /* OUT: Pointer to unused portion of zSql */
);

extern SQLITE_API int sqlite3_step_or(
  int what,
  sqlite3 *db,
  sqlite3_stmt *stmt
) ;

extern int sqlexecf_or_va(int what,sqlite3 *db,char *fmt,va_list va) ;
extern int sqlexecf_or(int what,sqlite3 *db,char *fmt,...) ;

extern int sqlpreparef_or_va(int what,sqlite3 *db,sqlite3_stmt **ppStmt,char *fmt,va_list va) ;
extern int sqlpreparef_or(int what,sqlite3 *db,sqlite3_stmt **ppStmt,char *fmt,...) ;

extern int sqlexecf_va(sqlite3 *db,char *fmt,va_list va) ;
extern int sqlexecf(sqlite3 *db,char *fmt,...) ;

extern int sql_single_row_int(sqlite3 *db,char *sql,int *v) ;

extern int sqlite3_bind_int_string(sqlite3_stmt *st,int i,char *s) ;
extern int sqlite3_bind_cstring(sqlite3_stmt *st,int i,char *s) ;

#endif


#ifdef __cplusplus /*Z*/
}
#endif
