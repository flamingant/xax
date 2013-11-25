#!perl -I../../.env

use envmain ;

# (perl-mode)
################################################################
__END__
# !!ghead()!!

set confirm off
set args --hexec=/timer --hexec=/irc
set print pretty
set non-stop off
set directories !!$devroot!!/pu/c

define done
set ufs.quit=1
c
end

define zps
set print elements 64
end

define zpl
set print elements 0
end

define stc
print stimer_clock_stop()
end

zps

!!gtail()!!
__ENDEND__
