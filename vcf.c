#ifdef __cplusplus /*Z*/
extern "C" {
#endif

#include	<string.h>

#include	"pu/type.h"
#include	"pu/mt.h"

#include	"vcf.h"
#include	"common.h"

extern int switch_state_read(char *cmd)
{
    if (!stricmp(cmd,"toggle") || 
	!strcmp(cmd,"^")) {
	return(VCF_SWITCH_TOGGLE) ;
	}
    if (!stricmp(cmd,"true") || 
	!stricmp(cmd,"yes") || 
	!stricmp(cmd,"on") || 
	!strcmp(cmd,"+")) {
	return(VCF_SWITCH_ON) ;
	}
    if (!stricmp(cmd,"false") || 
	!stricmp(cmd,"no") || 
	!stricmp(cmd,"off") || 
	!strcmp(cmd,"-")) {
	return(VCF_SWITCH_OFF) ;
	}
    return(VCF_UNKNOWN) ;
}

extern int switch_state_write(MT *mt,int st)
{
    if (st)
	 return mtputs(mt,"on") ;
    else return mtputs(mt,"off") ;
}

/* ================================================================ */
extern int vcf_switch_getchar(MT *mt,int st)
{
    return switch_state_write(mt,st) ;
    }

extern int vcf_switch_setchar(char *cmd)
{
    return switch_state_read(cmd) ;
}

/* ================================================================ */
#include	<stdlib.h>
#include	"log.h"
#include	".gen/vcf.h"

extern void vcf_apply(u32 (*fun)(void *,int,u32),void *c,int m,u32 a)
{
    MT	mt[1] ;
    MTALLOCA(mt,1024) ;
    char	*name ;
    char	*oldstate ;
    char	*newstate ;
    char	*mname = (char *) glget(glt_VCF_,(GLT_CAR_T) m) ;
    name = (char *) fun(c,VCF_GET_NAME,(u32) mt) ;
    oldstate = (char *) fun(c,VCF_GET_STATE,(u32) mt) ;
    fun(c,m,a) ;
    newstate = (char *) fun(c,VCF_GET_STATE,(u32) mt) ;
    log_printf(ls_generic,"%s apply %s: %s --> %s\n",name,mname,oldstate,newstate) ;
    }

#include	".gen/vcf.c"

#ifdef __cplusplus /*Z*/
}
#endif
