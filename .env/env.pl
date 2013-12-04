#!perl -Ie:/perl/lib

$| = 1 ;

use stdmain ;

$arg = {%$arg,
} ;

$cwd = getcwd ;

sub git_fake_ignore {
    my $f = [".bwg/genver.pm",
	     ".bwg/.gdb.el",
	     ".bln/genver.pm",
	     ".bln/.gdb.el",
	] ;
    chdir ".." ;
    system "git update-index --assume-unchanged " . join(" ",@$f) ;
}

sub main {
    sync ;
}

__END__
