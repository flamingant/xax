#ifdef __cplusplus /*Z*/
extern "C" {
#endif

#include	<stdio.h>

#include	<stdlib.h>

#include	<pu/iterator.h>
#include	<pu/rcons.h>

#include	"uf.h"
#include	"ufi.h"

#include	"common.h"

#define UF_ARG_WANT_ALL	1

#include	"uf_arg.h"

static void ufi_finish(UFIC *c)
{
    c->oma(c,UFIM_FINISH,0) ;
    }

extern void ufi_abort(UFIC *c,int reason)
{
    c->oma(c,UFIM_ABORT,reason) ;
    if (c->st)
	stimer_kill(c->st) ;
    ufi_finish(c) ;
    }

/* ================================================================ */
static void ufi_next(UFIC *c)
{
    if (!iterator_get_next(c->it,&c->next)) {
	c->oma(c,UFIM_LAST,0) ;
	c->st->state &= ~STS_REARM ;
	ufi_finish(c) ;
	}
    else {
	c->iti++ ;
	c->oma(c,UFIM_NEXT,(u32) c->next) ;
	}
    }

static u32 ufi_st_wakeup_next(STIMER *st,int m,u32 a)
{
    switch(m) {
    case STM_EXPIRE:
	ufi_next((UFIC *) st->c) ;
	break ;
	}
   return(0) ;
}

static u32 ufi_st_expire_first(STIMER *st,UFIC *c)
{
    c->st = stimer_add(ufi_st_wakeup_next,c,c->wait.between) ;
    stimer_expire_now(c->st) ;
    c->st->state |= STS_REARM ;
    c->oma(c,UFIM_FIRST,0) ;
    return 0 ;
    }

static u32 ufi_st_wakeup_first(STIMER *st,int m,u32 a)
{
    switch(m) {
    case STM_EXPIRE:
	return(ufi_st_expire_first(st,(UFIC *) st->c)) ;
	}
   return(0) ;
}

/* ================================================================ */
extern void ufi_start(UFIC *c)
{
    if (iterator_feature_supported(c->it,ITM_COUNT_TOTAL))
	 c->itn = iterator_count_total(c->it) ;
    else {
	errorshow("iterator doesn't support ITM_COUNT_TOTAL\n") ;
	c->itn = -1 ;
	}
    c->oma(c,UFIM_START,c->itn) ;
    if (c->itn == 0) {
	ufi_finish(c) ;
	return ;
    }
    c->iti = 0 ;
    iterator_rewind(c->it) ;
    c->st = stimer_add(ufi_st_wakeup_first,c,c->wait.before) ;
}

/* ================================================================ */
/* ufi is a special iterative UF which invokes a sequence
   of UF instances with a delay in between
   */

static u32 ufi_uff(UF *uf,int m,u32 a)
{
    UFIC *c = (UFIC *) uf->d.v ;
    switch(m) {
    case UFM_DESTROY:
	c = 0 ;
	return(0) ;
    case UFM_TYPENAME:
	return(mtputs((MT *) a,"UFI")) ;
    case UFM_SELECT_OK:
	break ;
    default:
	break ;
	}
    return(null_uff(uf,m,a)) ;
    }

/* ================================================================ */
#include	"http.h"

static u32 oma_z(UFIC *c,int m,u32 a)
{
    switch(m) {
    case UFIM_START:
 	printf("start\n") ;
	break ;
    case UFIM_FIRST:
	printf("first\n") ;
	break ;
    case UFIM_NEXT:
	printf("next %p\n",c->next) ;
	break ;
    case UFIM_LAST:
	printf("last\n") ;
	break ;
    case UFIM_FINISH:
	iterator_close(c->it) ;
	free(c->it->context.v) ;
	printf("done\n") ;
	break ;
    }
    fflush(stdout) ;
    return(0) ;
    }


static u32 ufi_test(HSR *r,u32 a)
{
    RCONS_ITERATOR_CONTEXT *icv = tscalloc(1,RCONS_ITERATOR_CONTEXT) ;
    UFIC	*c = tscalloc(1,UFIC) ;
    c->wait.before = 1000 ;
    c->wait.between = 2000 ;
    c->oma = oma_z ;
{
    icv->start = rcons_cons((RCONSVAL) "3",0) ;
    icv->start = rcons_cons((RCONSVAL) "2",icv->start) ;
    icv->start = rcons_cons((RCONSVAL) "1",icv->start) ;
    icv->start = rcons_cons((RCONSVAL) "0",icv->start) ;
    iterator_open(c->it,rcons_iterator_fun,icv) ;
    ufi_start(c) ;
    return 0 ;
    }
    }

/* ~# use http ; #~ */

/* ~~hpf(help => "Test the ufi feature")~~ */
static u32 hpf_ufi_test(HSR *r,int m,u32 a)
{
    switch (m) {
    case HPM_INTERNAL:
	return ufi_test(r,a) ;
    case HPM_CONTENT_SEND:
	hsr_printf(r,"<h2>OK!</h2>") ;
	break ;
    default:
	break ;
	}
    return(hpf_generic_ok(r,m,a)) ;
    }

#include	".gen/ufi.c"

#ifdef __cplusplus /*Z*/
}
#endif
