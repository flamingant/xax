#ifdef __cplusplus /*Z*/
extern "C" {
#endif

#ifndef __vcf_h
#define __vcf_h

typedef u32 VCFA ;

typedef u32 (*VCFFUN)(void *,int,VCFA) ;

typedef struct __struct_VCFT VCFT ;	/* VCF Type */
typedef struct __struct_VCFB VCFB ;	/* VCF Binding */

struct __struct_VCFT {
    char	*name ;
    int		(*decode)(VCFB *,char *) ;
    int		(*encode)(VCFB *,MT *) ;
    int		(*charspecial)(VCFB *,char *) ;
    } ;

struct __struct_VCFB {
    char	*name ;
    VCFT	*type ;
    void	*data ;
    } ;

/*
~# use decode ; #~
~~define_decode("VCF_(\\w+)")~~
*/

#define VCF_NOVALUE		-2
#define VCF_UNKNOWN		-1

#define VCF_GETINT		0
#define VCF_SETINT		1
#define VCF_GETCHAR		2
#define VCF_SETCHAR		3

#define VCF_SWITCH_ON		4
#define VCF_SWITCH_OFF		5
#define VCF_SWITCH_TOGGLE	6

#define VCF_GET_NAME		7
#define VCF_GET_STATE		8

#define VCF_CHARSPECIAL		9

extern VCFB *vcfb_lookup(VCFB *v,char *name) ;

extern VCFT vcft_int[] ;
extern VCFT vcft_bool[] ;

extern int switch_state_write(MT *mt,int st) ;
extern int switch_state_read(char *cmd) ;

extern int vcf_switch_getchar(MT *mt,int st) ;
extern int vcf_switch_setchar(char *cmd) ;

#ifndef VCFU_T_DEFINED

typedef union {
#ifdef __mt_h
    MT		*mt ;
#endif
    u32		u ;
    char	*s ;
#ifdef VCFU_EXTRA
    VCFU_EXTRA
#endif
    } VCFU ;

#endif

extern u32 irc_state_vcf(void *c,int m,u32 a) ;

#define DUMMY_VCC	0

extern void vcf_apply(u32 (*fun)(void *,int,u32),void *c,int m,u32 a) ;

#endif

#ifdef __cplusplus /*Z*/
}
#endif
