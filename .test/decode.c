/*
~!
use decode ;
!~
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

/* ~~define_decode(zzz())~~ */
