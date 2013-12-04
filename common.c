#ifdef __cplusplus /*Z*/
extern "C" {
#endif

#include	<stdlib.h>
#include	<string.h>

#include	"arg.h"
#include	"common.h"

GOPT gopt ;

/* ~# use arg ; #~ */

/* ~~argset(help => "General options")~~ */

/* ~~arg(help => "Add torrents in paused state")~~ */
static int argf__torrent_add_paused(char *name,char *value,void *a0)
{
    gopt.add_paused = strtol(value,0,10) ;
    return(ASF_ARGACCEPTED) ;
}

#include	".gen/common.c"

#ifdef __cplusplus /*Z*/
}
#endif
