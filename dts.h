#ifdef __cplusplus /*Z*/
extern "C" {
#endif

#ifndef __dts_h
#define __dts_h

typedef struct {
    char	year[4] ;
    char	hy1 ;
    char	month[2] ;
    char	hy2 ;
    char	day[2] ;
    char	sp1 ;
    char	hour[2] ;
    char	c1 ;
    char	minute[2] ;
    char	c2 ;
    char	second[2] ;
    char	dot ;
    char	msec[3] ;
    char	zero ;
} DATETIMESTRING ;

extern void dts_init(DATETIMESTRING *ts) ;
extern void dts_copy_hh_mm_ss(DATETIMESTRING *ts,char *s) ;

extern int dts_month_1(char *s) ;

#endif

#ifdef __cplusplus /*Z*/
}
#endif
