#~:standard main for command line and cgi - modified from e:/perl/lib/stdmain.pm

package stdmain ;

use Exporter ;
@ISA = ('Exporter') ;
@EXPORT = qw($arg $argx cgienc argspec call args_encode args_dump dp script_home) ;

use Getopt::Long ;

END {exit stdmain() ;}

$arg = {
    cmd		=> undef,
    debug	=> 0,
    BEGIN	=> undef,
    MAIN	=> undef,
    END		=> undef,
} ;

sub dp {
    my $level = shift ;
    if ($arg->{debug} < $level) {return ;}
    printf @_ ;
}

################################################################
=pod
QUERY_STRING_X is used to get around the 'fact' that cgi scripts are
not allowed to modify the QUERY_STRING or REQUEST_URI environment

an alternate method would be to pass an extra ENV variable to indicate
the QUERY_STRING has been 'proxied'

=cut

################################################################
sub script_home {
    (my $p = "$0") ;
    $p =~ s!/[^/]*$!! or return(".") ;
    $p ;
}

sub chdir_script_home {
    my $p = script_home ;
    chdir $p or die "cannot chdir to \"$p\"" ;
}

################################################################
sub cgiencall {
    my $s = join "",@_ ;
    my @s = split //,$s ;
    @s = map {sprintf("%%%02x",ord($_))} @s ;
    join "",@s ;
}

sub cgienc {
    my $s = join "",@_ ;
    my @s = split //,$s ;
    @s = map {ord($_) < 0x40 || ord($_) > 0x7f ? sprintf("%%%02x",ord($_)) : $_} @s ;
    join "",@s ;
}

sub cgidec {
    my $s = join "",@_ ;
    $s =~ s/\+/ /g ;
    $s =~ s/%(..)/chr(hex($1))/ge ;
    $s ;
}

sub env_dump {
    for (sort keys %ENV) {
	print "$_ => $ENV{$_}<p>\n" ;
    }
}

use Data::Dumper ;

sub args_dump {
    my $a = shift || $arg ;
    my $p = Data::Dumper->Dump([$a],["args"]);
    print $p ;
}

sub cgiargs_encode {
    my ($a) = (@_) ;
    join("&",map {"$_=" . $a->{$_}} keys %$a) ;
}

sub cgiargs_decode {
    my ($a,$q) = (@_) ;
    if (!(defined($ENV{REQUEST_METHOD}))) {return $a}
    if (uc($ENV{REQUEST_METHOD}) eq "POST") {
	read(STDIN,$q,$ENV{'CONTENT_LENGTH'});
	$a->{_post} = $q ;
    }
    else {
	$q ||= $ENV{QUERY_STRING_X} || $ENV{QUERY_STRING} || "" ;
	$a->{_get} = $q ;
    }
    $a->{_raw} = $q ;
    my @q = split(/&/,$q) ;
    my @qq ;
    for (@q) {
	@qq = split(/=/,$_) ;
	next if length $qq[1] == 0 ;
	$a->{$qq[0]} = cgidec $qq[1] ;
    }
    $a ;
}

sub call {
    my $s = shift ;
    $s or return 0 ;
    if (ref $s eq 'ARRAY') {
	my $r = 0 ;
	for (@$s) {
	    $r |= call($_,@_) ;
	} 
	return $r ;
    }
    elsif (ref $s eq 'CODE') {
	return $s->(@_) || 0 ;
    }
    else {
	my $fun = eval "\\&main::$s" ;
	$fun->(@_) or 0 ;
    }
}

sub argspec {
    my @argnames = ("argdump",keys %$arg)  ;
    my @argspec ;
    for (@argnames) {
	my $s = ":s" ;
	if (defined $argx->{$_}->{spec}) {
	    $s = $argx->{$_}->{spec} ;
	}
	push @argspec,"$_$s" ;
    }
    @argspec ;
}

sub args_encode {
    my @a ;
    my @keys = @_ ;
    for (@keys) {
	my $spec ;
	next unless defined $arg->{$_} ;
	$spec = $argx->{$_}->{spec} || ":s" ;
	if ($spec eq ":s") {
	    push @a,"--$_=$arg->{$_}" ;
	}
	elsif ($spec eq '!') {
	    push @a,"-$_" if $arg->{$_} ;
	}
    }
    @a ;
}

sub stdmain {
    my $fun ;
    my $query = $ENV{QUERY_STRING_X} || $ENV{QUERY_STRING} ;

    $arg->{cmd} ||= "main" ;
    
    my @argspec = argspec ;

# always try command line first

    Getopt::Long::Configure('pass_through') ;
    GetOptions($arg,@argspec) ;

# override with any cgi args

    $arg = cgiargs_decode($arg,$query) ;

    if ($arg->{argdump}) {args_dump() }

    my $r = 0 ;
    $r |= call $arg->{BEGIN} ;
    $r |= call $arg->{MAIN} ;
    $r |= call $arg->{cmd},@ARGV ;
    $r |= call $arg->{END} ;

    $r ;
}

################################################################
1 ;
