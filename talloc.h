#ifdef __cplusplus /*Z*/
extern "C" {
#endif

#ifndef __talloc_h
#define __talloc_h

#include	<malloc.h>

#define tsmalloc(t)	((t *) malloc(sizeof(t)))
#define tscalloc(n,t)	((t *) calloc(n,sizeof(t)))

#endif

#ifdef __cplusplus /*Z*/
}
#endif
