#include	"common.h"
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>

#include	<pu/rcons.h>

/* ================================================================ */
class ARGITEM ;
class ARGSET ;

class ARGITEM {
    public:
    friend ARGSET;

    char	*help ;
    char	*argname ;
    char	*valuename ;

    ARGITEM(ARGSET &set,
	    char *help_,
	    char *argname_,
	    char *valuename_,
	    int	(*accept)(char *,char *,void *)
	    ) ;

    virtual int match(char *name)
{
    return !strcmp(argname,name) ;
    }
    void prin()
{
    printf("%s %s\n",argname,valuename) ;
    }
static void print(ARGITEM &item)
{
    item.prin() ;
    }
static int match_(ARGITEM &item,char *name)
{
    return item.match(name) ;
    }

/* accept is not a class member function because it is different
   for every ARGITEM and so if it was virtual it would require a
   different subclass for every ARGITEM
   */

    int		(*accept)(char *,char *,void *) ;
} ;

/* ================================================================ */
class ARGSET {
    public:
    char *help ;
    ARGSET(char *help_) :
	help(help_)
	{
	printf("%s\n",help) ;
	} ;
    ~ARGSET() {
	rcons_free_list(items) ;
	printf("we are done %s\n",help) ;
	} ;
    void additem(ARGITEM *item) {
	items = rcons_cons(item,items) ;
	printf("%s\n",item->argname) ;
	}
    ARGITEM *itemfindbyname(char *name) {
	RCONS *rc ;
	for (rc = items ; rc ; rc = rc->cdr) {
	    ARGITEM *a = ((ARGITEM *) rc->car) ;
	    if (a->match(name)) return (a) ;
	    }
	return 0 ;
	}
    private:
    RCONS	*items ;

    public:
    void showitems() {
	rcons_do0((void (*)(P32)) ARGITEM::print,items) ;
	}
} ;

/* ================================================================ */
ARGITEM::ARGITEM(ARGSET &set,
		 char *help_,
		 char *argname_,
		 char *valuename_,
		 int	(*accept_)(char *,char *,void *)) :
    help(help_),
    argname(argname_),
    valuename(valuename_),
    accept(accept_)
{
    set.additem(this) ;
    printf("created ARGITEM\n") ;
}

/* ================================================================ */
class ARGITEM_M: ARGITEM {
    public:
    using ARGITEM::ARGITEM ;
    int match(char *name)
{
    return (argname[0] == name[0]) ;
    }
    } ;
/*    
http://stackoverflow.com/questions/12783643/force-a-derived-class-to-use-the-constructor-of-the-base-class
http://www.stroustrup.com/C++11FAQ.html#inheriting
*/
/* ================================================================ */
extern ARGSET as ;

static int argf__debug_mask(char *name,char *value,void *a0)
{
    return(1) ;
    }

static ARGITEM arg__zzz(as,
		      "Set main mode of program",
		      "m",
		      "MODE",
			argf__debug_mask
		      ) ;

static ARGITEM arg__yyy(as,
		      "Y",
		      "YY",
		      "YYY",
			argf__debug_mask
		      ) ;

static ARGITEM_M arg__ppp(as,
		      "P help",
		      "Proz",
		      "what value?",
			argf__debug_mask
		      ) ;

ARGSET as("Main") ;

static void arg(int argc,char **argv)
{
    as.showitems() ;
    for (int i = 0 ; i < argc ; i++) {
	if (!argv[i]) continue ;
	ARGITEM *item = as.itemfindbyname(argv[i]) ;
	if (item) {
	    item->prin() ;
	    item->accept(0,0,0) ;
	    }
	else {
	    printf("unknown arg %s\n",argv[i]) ;
	    }
	}
    printf("done\n") ;
    }

/* ================================================================ */
#include	"mainmode.h"

static int cpp_main(int argc,char **argv,MMC *c)
{
    arg(argc,argv) ;
    return(0) ;
    }

/* ================================================================ */
/* ~# use mainmode ; #~ *//* ~~mode("cpp",desc => "cpp entry point test")~~ */

#include	".gen/cpp.c"
