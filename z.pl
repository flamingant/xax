#!perl -Ie:/perl/lib

$| = 1 ;

use stdmain ;

$arg = {%$arg,
} ;

use File::Path ;
use timestamp ;
use Cwd ;

use Posix(strftime) ;

$cwd = getcwd ;

sub cppwrap {
    for my $f (glob("*.[ch]")) {
	open I,$f ;
	@lines = <I> ;
	close I ;
	if (grep m!extern "C"!,@lines) {
	    print "$_\n" ;
	}
	else {
	    unlink $f ;
	    open O,">$f" ;
	    print O "#ifdef __cplusplus /*Z*/\nextern \"C\" {\n#endif\n\n" ;
	    print O @lines ;
	    print O "\n#ifdef __cplusplus /*Z*/\n}\n#endif\n" ;
	}
    }
}

__END__
#	
	

