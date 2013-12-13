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
$root = "../../../.." ;

sub a {
    my $f = shift ;
    chdir $root ;
    system "perl ugen.pl gen $f" ;
    chdir $cwd ;
}

sub one {
    my $f = shift ;
    chdir $root ;
    system "perl ugen.pl gen $f >$cwd/$f.a" ;
    $c = file_contents $f ;
    $c =~ m!\(cg-end\)\*/! ;
    $c = $' ;
#    $c =~ m!^(static struct sym_init|static int mod_mimf)!m ;
    $c =~ m!^(MORSEL)!m ;
    $c = $` ;
    chdir $cwd ;
    open O,">$cwd/$f.b" ;
    binmode O ;
    print O $c ;
    close O ;
    system "diff --ignore-blank-lines $f.a $f.b" ;
}

sub _1209_1250 {
    chdir $root ;
    @f = glob("*.c") ;
    chdir $cwd ;
    for (@f) {
	print "$_:\n" ;
	one $_ ;
    }
}
sub _1209_1605 {
    one "lt_rect.c" ;
#    one "alloc.c" ;
#    one "print.c" ;
#    one "symcommon.c" ;
}

sub _1213_1149 {
    a "lt_rect.c" ;
    a "colorscale.c" ;
}

sub main {
}

__END__
