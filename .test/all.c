/* ~# use arg ; #~ */

/* ~~argset(name => "FOO",help => "argset 1")~~ */

/* ~~arg(help => "Set server document to directory")~~ */
static int argf__http_document_root(char *name,char *value,void *a0)
{
    g_hsg.document_root = strdup(value) ;
    return(ASF_ARGACCEPTED) ;
}

/* ~~arg(help => "foo")~~ */
static int argf__http_foo(char *name,char *value,void *a0)
{
    g_hsg.document_root = strdup(value) ;
    return(ASF_ARGACCEPTED) ;
/*~+(value => "override")+~*/
}

/* ~~argset(name => "BAR",help => "argset 2")~~ */

/* ~~arg(value => "prelude")~~ */
static int argf__bubbo(char *name,char *value,void *a0)
{
    return(ASF_ARGACCEPTED) ;
}
/* ================================================================ */
/*
~#
use decode ;
#~
~~enum_decode("UFM")~~
*/

typedef enum __enum_UFM {
    UFM_CREATE,
    UFM_DESTROY,
    UFM_POST_NOTIFY,
    UFM__STD_AFTERLAST,

    UFM__HT_FIRST	= 100,

    UFM_HT_RESPONSE_HDR	= UFM__HT_FIRST,
    UFM_HT_RESPONSE_BODY,
    UFM_HT_RESPONSE_JSON,
    UFM_HT_TORRENT_STATUS_GET,
    UFM_HT_NOTREADY,

    UFM__HT_AFTERLAST,
    } UFM ;

#define MTC_TRACKEVENT		1

#define RC_SYNTAX_ERROR		1
#define RC_VARLEN_ERROR		2
#define RC_TRACK_OVERRUN	3
#define RC_BAD_EVENT_CODE	4

/* ~~define_decode("OR_\\w+",file => "decode-or.h")~~ */
/* ~~define_decode("RC_\\w+")~~ */

/* test eval failure */
