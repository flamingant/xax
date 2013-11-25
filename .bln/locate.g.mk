#!perl -I../.env

use envmain ;

# (perl-mode)
################################################################
__END__
# !!ghead()!!

##
## HOST: !!$host!!
##

DOTEXE =	!!$exec_suffix!!

DEVDRIVE =	!!$drive!!
DEVROOT =	!!$devroot!!
SQLITE =	!!$sqlite!!
LIB_JANSSON =   !!$jansson!!

!!gtail()!!
__ENDEND__
