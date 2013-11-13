#!d:/p/perl/bin/perl -I.. -Ie:/daily/0911/25/marigold -Ie:/perl/lib/fly -Ie:/perl/lib

$| = 1 ;

use stdmain ;
use stdcgi ;

$arg = {
    cmd		=> "main",
    xxx		=> 1,
} ;

sub main {
    start_text ;
    print "method = $ENV{REQUEST_METHOD}<p>" ;
    print "POST = $arg->{_post}<p>" ;
    print "cmd = $arg->{cmd}<p>xxx = $arg->{xxx}" ;
}
