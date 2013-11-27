#ifndef __mainmode_h
#define __mainmode_h	1

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __type_h
#include	<pu/type.h>
#endif

typedef struct {
    char	*mode ;
    int		version_show ;
    int		help ;
    int		noinit ;
    } MMC ;

typedef struct struct_MMO MMO ;

typedef u32 (*MMF)(MMO *,int,u32) ;

#define MMM_GET_ENTRY		0
#define MMM_GET_NAME		1
#define MMM_MATCH_NAME		2
#define MMM_GET_HELP		3
#define MMM_GET_DESC		4
#define MMM_GET_ARGITEMS	5

typedef int (*MMENTRY)(int,char **,MMC *) ;

struct struct_MMO {
    void	*c ;
    MMENTRY	entry ;
    MMF		f ;
    char	*name ;
    char	*desc ;
    int		default_mmo ;
    int		enabled ;
    } ;
    
extern u32 mmf_null(MMO *o,int m,u32 a) ;

#define mmo_getname(m) ((char *) (m)->f(m,MMM_GET_NAME,0))
#define mmo_getdesc(m) ((char *) (m)->f(m,MMM_GET_DESC,0))

#ifdef __cplusplus
}
#endif
#endif
