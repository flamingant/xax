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
    $c = $' ;
    $c =~ m!^static!m ;
    $c = $` ;
    chdir $cwd ;
    open O,">$cwd/$f.b" ;
    binmode O ;
    print O $c ;
    close O ;
    system "diff $f.a $f.b" ;
}

sub _1209_1250 {
#    one "lt_rect.c" ;
    chdir "../../.." ;
    @f = glob("*.c") ;
    chdir $cwd ;
    for (@f) {
	print "$_:\n" ;
	one $_ ;
    }
}

sub main {
}

__END__
