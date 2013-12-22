

package common ;

use Exporter ;
@ISA = ('Exporter') ;
@EXPORT = qw(trim sprin quickdump cfparse dump_items) ;
@EXPORT = (@EXPORT,qw(hyphenate unhyphenate)) ;
@EXPORT = (@EXPORT,qw(hslice)) ;
@EXPORT = (@EXPORT,qw(lparse lprops)) ;

################################################################
sub lparse {
    my $s = shift ;
    my $r = [extract_multiple($s,
			      [sub {extract_bracketed($_[0],'()')}],
			      undef,
			      1
	     )] ;
    $r ;
}

sub lprops {
    my $s = shift ;
    my $p ;
    my $r = lparse $s ;
    for (@$r) {
	s!^.!!s ;
	s!.$!!s ;
	my ($k,$v) = split(/\s+/,$_,2) ;
	$p->{$k} = $v || "" ;
    }
    $p ;
}
    
################################################################
sub hslice ($@) {
    my $h = shift ;
    my @z = map {defined $h->{$_} ? ($_ => $h->{$_}) : ()} @_ ;
    @z ;
}

################################################################
sub trim {
    $_[0] =~ s!^\s+!! ;
    $_[0] =~ s!\s+$!! ;
    $_[0] ;
}

=pod

sprintf "%-10s,","foo" -> "foo       ,"
sprin   "%-10s#,#","foo" -> "foo,      "

sprin is like sprintf, but successive fragments enclosed in '#' pairs
are appended to the items to be formatted before sprintf is called.
The format elements are not adjusted for the added text (this is hard to do
and not obviously a good idea).
Used to provide nicer visual effect when printing variable length data
in lists

=cut

sub sprin {
    my $format = shift ;
    my @a ;
    while ($format =~ s!#(.*?)#!!) {
	my $a = shift ;
	$a = "$a$1" ;
	push @a,$a ;
    }
    sprintf($format,@a,@_) ;
}

use Data::Dumper ;

sub quickdump {
    my $dump = Data::Dumper->new([$_[0]]) ;
    print $dump->Terse(1)->Indent(1)->Dump ; ;
}

################################################################
use Text::Balanced(qw(extract_bracketed 
		      extract_codeblock
		      extract_delimited
		      extract_multiple
		      extract_variable
)) ;

sub cfparse {
    my $s = shift ;
    my $r = {} ;
    $s =~ s!(static|extern)(.*?)(?=[\{;])!!s ;
    $r->{dec} = "$1$2" ;
    $r->{dec} =~ s!/\*.*?\*/! !gs ;
    $r->{dec} =~ s!\s+! !gs ;
    $r->{dec} =~ s!\s*$!! ;

    $s =~ s!.*?(?=[\{;])!!s ;
    if ($s =~ m!^;!) {
	# extern or forward declaration - no body
    }
    else {
	$r->{body} = extract_bracketed($s,'{}"','\s*') ;
    }

    my $d = $r->{dec} ;
    $d =~ s!(\w+)\s*(?=\()! ! ;

    $r->{name} = $1 ;
    $d =~ s!(static|extern)\s*!! ;
    $r->{class} = $1 ;

    $d =~ s!\s*(\w+)(\s*\*+|)\s*!! ;
    $r->{type} = "$1$2" ;

    $d =~ s!^\s*\(!! ;
    $d =~ s!\s*\)\s*$!! ;

    while ($d) {
	my $a = {} ;
	if ($d =~ s!\s*(\w+)(\s*\*+|)!!) {
	    $a->{type} = "$1$2" ;
	    if (my $bf1 = extract_bracketed($d,'()')) {
		my $bf2 = extract_bracketed($d,'()') ;
		$bf1 =~ m!\w+! ;
		$a->{name} = $& ;
		$a->{nameform} = "$bf1$bf2" ;
	    }
	    elsif ($d =~ s!\w+!!) {
		$a->{name} = $& ;
	    }
	}
	else {
	    last ;
	}
	$d =~ s!,!! ;
	if ($a->{type} ne 'void') {
	    push @{$r->{args}},$a ;
	}
    }
    $r ;
}

################################################################
sub dump_items {
    my $items = shift ;
    my $dump = Data::Dumper->new([$items]) ;
    my $i = $dump->Terse(1)->Indent(1)->Dump ;
    $i ;
}

################################################################
sub hyphenate($) {
    local $_ = shift ;
    tr!_!-! ;
    $_ ;
}

sub unhyphenate($) {
    local $_ = shift ;
    tr!-!_! ;
    $_ ;
}

1 ;
