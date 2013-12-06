#ifdef __cplusplus /*Z*/
extern "C" {
#endif

#include	"dts.h"
#include	<string.h>
#include	<stdlib.h>

#include	"npu_util.h"

extern void dts_init(DATETIMESTRING *ts)
{
    strcpy(ts->year,"0000-00-00 00:00:00.000") ;
    }

extern void dts_copy_hh_mm_ss(DATETIMESTRING *ts,char *s)
{
    ts->hour[0]		= s[0] ;
    ts->hour[1]		= s[1] ;
    ts->minute[0]	= s[3] ;
    ts->minute[1]	= s[4] ;
    ts->second[0]	= s[6] ;
    ts->second[1]	= s[7] ;
    }

extern int dts_month_1(char *s)
{
    char	*us = (char *) alloca(strlen(s)+10) ;
    strcpy_lc(us,s) ;
    us[3] = 0 ;
    if (!strcmp("jan",us)) return(1) ;
    if (!strcmp("feb",us)) return(2) ;
    if (!strcmp("mar",us)) return(3) ;
    if (!strcmp("apr",us)) return(4) ;
    if (!strcmp("may",us)) return(5) ;
    if (!strcmp("jun",us)) return(6) ;
    if (!strcmp("jul",us)) return(7) ;
    if (!strcmp("aug",us)) return(8) ;
    if (!strcmp("sep",us)) return(9) ;
    if (!strcmp("oct",us)) return(10) ;
    if (!strcmp("nov",us)) return(11) ;
    if (!strcmp("dec",us)) return(12) ;
    return(-1) ;
    }


#ifdef __cplusplus /*Z*/
}
#endif
