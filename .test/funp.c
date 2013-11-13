/* a function returning a pointer to a function which is 
   not typedef'ed is complicated.
   The functions own parameters are inside and the parameters
   the returned function takes are on the outside
*/

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
    
