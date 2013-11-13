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
	    char *valuename_) ;

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
} ;

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
		 char *valuename_) :
    help(help_),
    argname(argname_),
    valuename(valuename_)
{
    set.additem(this) ;
    printf("created ARGITEM\n") ;
}

/* ================================================================ */
extern ARGSET as ;

static ARGITEM arg__zzz(as,
		      "Set main mode of program",
		      "m",
		      "MODE"
		      ) ;

static ARGITEM arg__yyy(as,
		      "Y",
		      "YY",
		      "YYY"
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
	    }
	else {
	    printf("unknown arg %s\n",argv[i]) ;
	    }
	}
    printf("done\n") ;
    }

/* ================================================================ */
static int cpp_main(int argc,char **argv,char *mode)
{
    arg(argc,argv) ;
    return(0) ;
    }

/* ================================================================ */
/* ~# use mainmode ; #~ *//* ~~mode("cpp",desc => "cpp entry point test")~~ */

#include	".gen/cpp.c"
