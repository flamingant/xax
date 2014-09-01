#ifdef __cplusplus /*Z*/
extern "C" {
#endif

#ifndef __do_h
#define __do_h

#include	<stdio.h>

typedef struct {
    int		level ;
    int		mask ;
    FILE	*fh ;
    } GDO ;

extern GDO gdo ;

extern void doph(char *p,int n) ;

extern void dof(char *fmt,...) ;
extern void dofn(int level,char *fmt,...) ;
extern void dofm(int mask,char *fmt,...) ;

extern void dof_init(char *file,int level,int mask) ;
extern void dof_file_set(char *file) ;
extern void dof_level_set(int) ;
extern void dof_mask_set(int) ;

#endif

#ifdef __cplusplus /*Z*/
}
#endif
