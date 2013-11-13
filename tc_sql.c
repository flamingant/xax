#ifdef __cplusplus /*Z*/
extern "C" {
#endif

#include	"sql.h"
#include	"tod.h"
#include	"tc_sql.h"

#include	<string.h>
#include	<stdlib.h>

#include	"common.h"

/* ================================================================ */
static SQLGP g ;

static int lsql(int what,char *sql)
{
    return(sqlite3_exec_or(what,g.dbh,sql,0,0,0)) ;
    }

extern SQLGP *sqlgp()
{
    return(&g) ;
    }

extern void transaction_begin(void)
{
    sqlite3_exec_or(OR_WARN,g.dbh,"begin",0,0,0) ;
    }

extern void transaction_end(void)
{
    sqlite3_exec_or(OR_WARN,g.dbh,"end",0,0,0) ;
    }

/* ================================================================ */
static void tannounce_create_table(void)
{
    lsql(OR_DIE,
"create table if not exists tannounce (\
id integer primary key,\
time_seen datetime,\
artist text,\
title text,\
year text,\
type text,\
format text,\
gid integer,\
tags text\
)") ;
}

static void tanno_ins_prepare(void)
{
    char *s = "insert or replace into tannounce values(?,?,?,?,?,?,?,?,?)" ;

    sqlite3_prepare_v2_or(OR_DIE,
			  g.dbh,
			  s,-1,
			  &g.tanno.s_ins,
			  0) ;
    }

static void tanno_select_prepare(void)
{
    char *s = "select id,time_seen,artist,title,year,type,format,gid,tags from tannounce where id = ?" ;

    sqlite3_prepare_v2_or(OR_DIE,
			  g.dbh,
			  s,-1,
			  &g.tanno.s_select,
			  0) ;
    }

extern int tanno_query_step(sqlite3_stmt *s,TANNO *t,void (*fun)(TANNO *,u32),u32 a)
{
    int		r ;
    r = sqlite3_step_or(OR_WARN,g.dbh,s) ;
    if (r != SQLITE_ROW) return r ;

    t->tid	 	= sqlite3_column_int(s,0) ;
    t->time_seen 	= (char *) sqlite3_column_text(s,1) ;
    t->artist 		= (char *) sqlite3_column_text(s,2) ;
    t->title 		= (char *) sqlite3_column_text(s,3) ;
    t->year 		= (char *) sqlite3_column_text(s,4) ;
    t->type 		= (char *) sqlite3_column_text(s,5) ;
    t->format 		= (char *) sqlite3_column_text(s,6) ;
    t->gid 		= sqlite3_column_int(s,7) ;
    t->tags 		= (char *) sqlite3_column_text(s,8) ;

    fun(t,a) ;

    sqlite3_reset(s) ;
    return r ;
    }

extern int tanno_query(TANNO *t,void (*fun)(TANNO *,u32),u32 a)
{
    sqlite3_stmt *s = g.tanno.s_select ;
    sqlite3_bind_int(s,1,t->tid) ;
    return(tanno_query_step(s,t,fun,a)) ;
    }

extern int tanno_add(TANNO *t)
{
    int		r ;
    sqlite3_stmt *s = g.tanno.s_ins ;
    r = sqlite3_bind_int(s,1,t->tid) ;
    sqlite3_bind_cstring(s,2,t->time_seen) ;
    sqlite3_bind_cstring(s,3,t->artist) ;
    sqlite3_bind_cstring(s,4,t->title) ;
    sqlite3_bind_cstring(s,5,t->year) ;
    sqlite3_bind_cstring(s,6,t->type) ;
    sqlite3_bind_cstring(s,7,t->format) ;
    sqlite3_bind_int(s,8,t->gid) ;
    sqlite3_bind_cstring(s,9,t->tags) ;
    r = sqlite3_step_or(OR_WARN,g.dbh,s) ;
    sqlite3_reset(s) ;
    return r ;
    }

/* ================================================================ */
static void tstatus_create_table(void)
{
    lsql(OR_DIE,
"create table if not exists tstatus (\
hash text primary key,\
id integer,\
queue integer,\
name text,\
total_size integer,\
state integerl,\
progress real,\
num_seeds integer,\
total_seeds integer,\
num_peers integer,\
total_peers integer,\
download_payload_rate integer,\
upload_payload_rate integer,\
eta integer,\
ratio real,\
distributed_copies real,\
is_auto_managed bool,\
time_added real,\
tracker_host text,\
save_path text,\
total_done integer,\
total_uploaded integer,\
max_download_speed integer,\
max_upload_speed integer,\
seeds_peers_ratio real,\
label text\
)") ;
    lsql(OR_DIE,
"create index if not exists tstatid ON tstatus(id)") ;
}

static void tstat_select_prepare(void)
{
    char *s = "select * from tstatus where hash = ?" ;
    sqlite3_prepare_v2_or(OR_DIE,g.dbh,s,-1,&g.tstat.s_select,0) ;
    }

static void tstat_ins_prepare(void)
{
    char *s = "insert into tstatus (hash,id) values(?,?)" ;
    sqlite3_prepare_v2_or(OR_DIE,g.dbh,s,-1,&g.tstat.s_ins,0) ;
    }

extern int torrent_id_to_hash(int id,char *hash)
{
    int		sr ;
    sqlite3_stmt *ss ;
    MT mtq[1] ;
    MTALLOCA(mtq,1024) ;
    mtprintf(mtq,"select hash from tstatus where id = %d",id) ;
    sr = sqlite3_prepare_v2_or(OR_DIE,g.dbh,mtq->s,-1,&ss,0) ;
    sr = sqlite3_step(ss) ;
    sqlite3_finalize(ss) ;
    if (sr == SQLITE_ROW) {
	if (hash) strcpy(hash,(const char *) sqlite3_column_text(ss,0)) ;
	return 1 ;
	}
    return 0 ;
    }

static void tstat_init(void)
{
    tstatus_create_table() ;
    tstat_select_prepare() ;
    tstat_ins_prepare() ;
    }

extern int db_tsi_load(TOD *tod)
{
    int		r ;
    sqlite3_stmt *ss,*sc ;
    ss = g.tstat.s_select ;
    sc = g.tstat.s_ins ;
    sqlite3_bind_cstring(ss,1,tod->hash) ;
    r = sqlite3_step(ss) ;
    if (r == SQLITE_ROW) {
	return 1 ;
	}
    else {
	return 0 ;
	}
    }

extern void db_tsi_load_or_create(TOD *tod)
{
    int		r ;
    sqlite3_stmt *ss,*sc ;
    ss = g.tstat.s_select ;
    sc = g.tstat.s_ins ;
    sqlite3_bind_cstring(ss,1,tod->hash) ;
    r = sqlite3_step(ss) ;
    if (r == SQLITE_ROW) {
	}
    else {
	sqlite3_bind_cstring(sc,1,tod->hash) ;
	sqlite3_bind_int(sc,2,tod->tid) ;
	r = sqlite3_step(sc) ;
	sqlite3_reset(sc) ;
	}
    sqlite3_reset(ss) ;
    }

/* ================================================================ */
extern void db_anno_load_or_create(TOD *tod)
{
    int		r ;
    sqlite3_stmt *ss,*sc ;
    ss = g.tstat.s_select ;
    sc = g.tstat.s_ins ;
    sqlite3_bind_cstring(ss,1,tod->hash) ;
    r = sqlite3_step(ss) ;
    if (r == SQLITE_ROW) {
	}
    else {
	sqlite3_bind_cstring(sc,1,tod->hash) ;
	sqlite3_bind_int(sc,2,tod->tid) ;
	r = sqlite3_step(sc) ;
	sqlite3_reset(sc) ;
	}
    sqlite3_reset(ss) ;
    }

/* ================================================================ */
static void session_table_create(void)
{
    lsql(OR_DIE,
"create table if not exists sessions (\
id integer primary key autoincrement,\
start datetime,\
finish datetime\
)") ;
}

static void session_state_table_create(void)
{
    lsql(OR_DIE,
"create table if not exists session_state (\
session integer,\
file text,\
module text,\
name text,\
value text\
)") ;
}

static void session_start(void)
{
    int e ;
    e = sqlexecf(g.dbh,
		"insert into sessions values(NULL,datetime('now'),NULL)") ;

    g.sid = sqlite3_last_insert_rowid(g.dbh) ;
}

static void session_finish(void)
{
    sqlexecf_or(OR_DIE,g.dbh,
		"update sessions set finish = datetime('now') where id = %d",
		g.sid) ;
}

static void session_start_new(void)
{	
    session_finish() ;
    session_start() ;
    }

/* ================================================================ */
static void table_exist_verify(void)
{
    /* query table with
       select count(*) from sqlite_master where type = 'table' and name = 'session' ;
       */
    session_table_create() ;
    session_state_table_create() ;
    tannounce_create_table() ;
    }

/* ================================================================ */
#include	"uf.h"
#include	"mag.h"

typedef struct {
    const uchar *module ;
    const uchar *name ;
    const uchar *value ;
    sqlite3_stmt	*stmt ;
    } OPTIONROW ;

typedef struct struct_OPTIONSAVESTATE OPTIONSAVESTATE ;

struct struct_OPTIONSAVESTATE {
    void		(*save)(OPTIONSAVESTATE *,char *,char *,int,u32) ;
    sqlite3		*dbh ;
    sqlite3_stmt	*st_delete ;
    sqlite3_stmt	*st_insert ;
    } ;

static int opt_try(UF *u,OPTIONROW *row)
{
    return(uf_send(u,UFM_OPTION_TRY,(u32) row)) ;
    }

static void dispatch_option(OPTIONROW *row)
{
    UF		*u ;
    u = uf_scan1_rcons(magpub()->module_uf_list,(UFSCANFUN1) opt_try,(u32) row) ;
    }

/* session 0 is a duplicate of the previous sessions saved settings */

#define SQL_Q_LOAD	"select module,name,value from session_state where session = ? and file = 'default'"
#define SCQ_MODULE	0
#define SCQ_NAME	1
#define SCQ_VALUE	2

extern int session_settings_load(int session)
{
    OPTIONROW row[1] ;
    int		n = 0 ;
    sqlite3_stmt *stmt ;
    sqlite3_prepare_v2_or(OR_DIE,g.dbh,SQL_Q_LOAD,-1,&stmt,0) ;
    sqlite3_bind_int(stmt,1,session) ;
    row->stmt = stmt ;
    while (1) {
	int rc = sqlite3_step(stmt) ;
	if (rc == SQLITE_DONE) {
	    sqlite3_finalize(stmt) ;
	    break ;
	}
	else if (rc == SQLITE_ROW) {
	    n++ ;
	    row->module = sqlite3_column_text(stmt,SCQ_MODULE) ;
	    row->name = sqlite3_column_text(stmt,SCQ_NAME) ;
	    row->value = sqlite3_column_text(stmt,SCQ_VALUE) ;
	    dispatch_option(row) ;
	}
    }
    return(n) ;
    }

static void previous_session_settings_load(void)
{
    session_settings_load(0) ;
    }

extern void session_settings_save(void)
{
    }

/* ================================================================ */
#include	<pu/exithook.h>

static void sql_finish(void *a)
{
    if (!g.dbh) return ;
    session_settings_save() ;
    sqlite3_finalize(g.tanno.s_ins) ;
    sqlite3_finalize(g.tanno.s_select) ;
    sqlite3_finalize(g.tstat.s_ins) ;
    sqlite3_finalize(g.tstat.s_select) ;
    sqlite3_close(g.dbh) ;
    sqlite3_shutdown() ;
    g.dbh = 0 ;

    }

extern void sql_init(void)
{
    char *dbname = "data/tc.db" ;
    sqlite3_open_v2_or(OR_DIE,dbname,&g.dbh,
		       SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE,NULL);
    table_exist_verify() ;
    tanno_ins_prepare() ;
    tanno_select_prepare() ;
    tstat_init() ;
    session_start() ;
    previous_session_settings_load() ;
    exithook_install((EXITHOOK) sql_finish,0) ;
}

#ifdef __cplusplus /*Z*/
}
#endif
