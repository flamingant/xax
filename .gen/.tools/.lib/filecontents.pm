package filecontents ;

use Exporter ;
@ISA = ('Exporter') ;
@EXPORT = qw(file_contents_ra file_contents_a file_contents file_write file_cache_cmd) ;

sub file_contents_ra {
    return undef if !(-f $_[0]) || !(-r $_[0]) || !(-s $_[0]) ;
    open FCI,$_[0] ;
    binmode FCI unless $_[1] ;
    my $r = [<FCI>] ;
    close FCI ;
    $r ;
}

sub file_contents_a {
    return undef if !(-f $_[0]) || !(-r $_[0]) || !(-s $_[0]) ;
    open FCI,$_[0] ;
    binmode FCI unless $_[1] ;
    my @r = <FCI> ;
    close FCI ;
    @r ;
}

sub file_contents {
    return undef if !(-f $_[0]) || !(-r $_[0]) || !(-s $_[0]) ;
    open FCI,$_[0] ;
    binmode FCI unless $_[1] ;
    my @r = <FCI> ;
    close FCI ;
    join "",@r ;
}

sub file_write {
    my ($file) = (shift) ;
    open FCO,">$file" or die "$! on write file $file" ;
    binmode FCO ;
    print FCO @_ ;
    close FCO ;
    "@_" ;
    }

sub file_cache_cmd {
    my $cf = shift ;
    my $cmd = shift ; ;
    if (-f $cf) {
	print "cached\n" ;
	open I,$cf ;
	@r = <I> ;
	close I ;
    }
    else {
	print "running\n" ;
	open I,"sh -c \"$cmd\"|" ;
	@r = <I> ;
	open O,">$cf" ;
	print O @r ;
	close O ;
	close I ;
    }
    @r ;
    }

1 ;
