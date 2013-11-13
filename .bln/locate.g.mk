#!perl -I../.env

use envmain ;

# (perl-mode)
################################################################
__END__
# !!ghead()!!

##
## HOST: !!$host!!
##

DEVDRIVE =	!!$drive!!
DEVROOT =	$(DEVDRIVE)/.master/
SQLITE =	!!$sqlite!!
LIB_JANSSON =   !!$jansson!!
AEXE =		tc!!$exec_suffix!!

!!gtail()!!
__ENDEND__
