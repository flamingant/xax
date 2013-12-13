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
    $c =~ m!^(static struct sym_init|static int mod_mimf)!m ;
    $c = $` ;
    chdir $cwd ;
    open O,">$cwd/$f.b" ;
    binmode O ;
    print O $c ;
    close O ;
    system "diff --ignore-blank-lines $f.a $f.b" ;
}

sub _1209_1250 {
    chdir "../../.." ;
    @f = glob("*.c") ;
    chdir $cwd ;
    for (@f) {
	print "$_:\n" ;
	one $_ ;
    }
}
sub _1209_1605 {
#    one "lt_rect.c" ;
    one "alloc.c" ;
}

sub main {
}

__END__
