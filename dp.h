#ifdef __cplusplus /*Z*/
extern "C" {
#endif

#ifndef __dp_h
#define __dp_h

#include	<stdio.h>

typedef struct {
    int		level ;
    int		mask ;
    FILE	*fh ;
    } GDBG ;

extern GDBG gdbg ;

extern void dph(char *p,int n) ;

extern void dpf(char *fmt,...) ;
extern void dpfn(int level,char *fmt,...) ;
extern void dpfm(int mask,char *fmt,...) ;

extern void dpf_init(char *file,int level,int mask) ;
extern void dpf_file_set(char *file) ;
extern void dpf_level_set(int) ;
extern void dpf_mask_set(int) ;

extern int dp_arg_try(char *name,char *value) ;

#endif

#ifdef __cplusplus /*Z*/
}
#endif
