#ifdef __cplusplus /*Z*/
extern "C" {
#endif

#ifndef __tc_sql_h
#define __tc_sql_h	1

#include	"sql.h"

extern void sql_init(void) ;

typedef struct {
    sqlite3	*dbh ;
    int		sid ;
    struct {
	sqlite3_stmt *s_ins ;
	sqlite3_stmt *s_select ;
	} tanno ;
    struct {
	sqlite3_stmt *s_ins ;
	sqlite3_stmt *s_select ;
	} tstat ;
    } SQLGP ;

extern SQLGP *sqlgp() ;

typedef struct {
    char	*artist ;
    char	*title ;
    char	*year ;
    char	*type ;
    char	*format ;
    int		gid ;
    int		tid ;
    char	*tags ;
    char	*time_seen ;
    } TANNO ;

extern int tanno_add(TANNO *) ;
extern int tanno_query(TANNO *t,void (*fun)(TANNO *,u32),u32 a) ;

extern void db_tsi_load_or_create(TOD *) ;
extern int db_tsi_load(TOD *) ;

extern void transaction_begin(void) ;
extern void transaction_end(void) ;

extern int torrent_id_to_hash(int id,char *hash) ;

#endif


#ifdef __cplusplus /*Z*/
}
#endif
