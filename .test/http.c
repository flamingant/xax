/* ~! use http ; !~ */

/* ================================================================ */
/* ~~hpf("quit")~~ */
static u32 hpf_quit(HSR *r,int m,u32 a)
{
    switch (m) {
    case HPM_GET_NAME:
	return((u32) "quit") ;
    case HPM_CONTENT_SEND:
	hsr_printf(r,"<h2>Bye!</h2>") ;
	ufs_quit() ;
	break ;
    default:
	break ;
	}
    return(hpf_generic_ok(r,m,a)) ;
    }

/* ================================================================ */
/* ~~hpf("favicon")~~ */
static u32 hpf_favicon
(
 HSR *(*r)(int,int),
 int m,
 u32 *a
)
{
    switch (m) {
    case HPM_GET_NAME:
	return((u32) "favicon.ico") ;
    case HPM_HEADER_SEND:
	r->http.status = 302 ; 
	hsr_send_status(r) ;
	hsr_printf(r,"Location: http://boo%s\r\n\r\n",r->url->s) ;
	return(0) ;
    default:
	break ;
	}
    return(hpf_null(r,m,a)) ;
    }

/* a function returning a pointer to a function which is 
   not typedef'ed is complicated.
   The functions own parameters are inside and the parameters
   the returned function takes are on the outside
*/

/* ~~hpf()~~ */
static int f(int x)
{
    return 0 ;
    }

static int v(void)
{
    return 0 ;
    }

static int (*foo(int x))(void)
{
    return v ;
    }

static int t(void)
{
    void *f = foo(1) ;
    }
    
