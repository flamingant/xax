#ifdef __cplusplus /*Z*/
extern "C" {
#endif

#include	<string.h>

#include	"pu/type.h"
#include	"pu/mt.h"

#include	"vcf.h"
#include	"npu_util.h"

extern int switch_state_read(char *cmd)
{
    if (!cmd || !*cmd) return VCF_NOVALUE ;
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
    /* mt is used to store fragments rather than one contiguous string */
    mtputc(mt,0) ;
    oldstate = (char *) fun(c,VCF_GET_STATE,(u32) mt) ;
    mtputc(mt,0) ;
    fun(c,m,a) ;
    newstate = (char *) fun(c,VCF_GET_STATE,(u32) mt) ;
    log_printf(ls_generic,"%s apply %s: %s --> %s\n",name,mname,oldstate,newstate) ;
    }

/* ================================================================ */
extern VCFB *vcfb_lookup(VCFB *v,char *name)
{
    if (!name) return 0 ;
    while (v->name && strcmp(v->name,name)) v++ ;
    if (!v->name) return 0 ;
    return v ;
    }

/* ================================================================ */
extern int vcft_bool_decode(VCFB *b,char *s)
{
    return 0 ;
    }

extern int vcft_bool_encode(VCFB *b,MT *mt)
{
    int		*p = b->data ;
    return mtputs(mt,*p ? "on" : "off") ;
    }

extern int vcft_bool_charspecial(VCFB *v,char *s)
{
    if (!stricmp(s,"toggle") || 
	!strcmp(s,"^")) {
	return(VCF_SWITCH_TOGGLE) ;
	}
    if (!stricmp(s,"true") || 
	!stricmp(s,"yes") || 
	!stricmp(s,"on") || 
	!strcmp(s,"+")) {
	return(VCF_SWITCH_ON) ;
	}
    if (!stricmp(s,"false") || 
	!stricmp(s,"no") || 
	!stricmp(s,"off") || 
	!strcmp(s,"-")) {
	return(VCF_SWITCH_OFF) ;
	}
    return(VCF_UNKNOWN) ;
}

VCFT vcft_bool[] = {
    "bool",
    vcft_bool_decode,
    vcft_bool_encode,
    vcft_bool_charspecial} ;

/* ================================================================ */
VCFT vcft_int[] = {
    "int"
} ;

/* ================================================================ */
#include	".gen/vcf.c"

#ifdef __cplusplus /*Z*/
}
#endif
