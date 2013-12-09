#!perl -Ie:/perl/lib

$| = 1 ;

use stdmain ;

$arg = {%$arg,
} ;

use File::Path ;
use filecontents ;
use timestamp ;
use Cwd ;

use Posix(strftime) ;

$cwd = getcwd ;

sub one {
    my $f = shift ;
    chdir "../../.." ;
    system "perl ugen.pl gen $f >$cwd/$f.a" ;
    $c = file_contents $f ;
    $c =~ m!\(cg-end\)\*/! ;
    chdir $cwd ;
    open O,">$cwd/$f.b" ;
    binmode O ;
    print O $' ;
    close O ;
    system "diff $f.a $f.b" ;
}

sub _1209_1250 {
    one "lt_rect.c" ;
}

sub main {
}

__END__
