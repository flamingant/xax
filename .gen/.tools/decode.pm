=pod

This module has a different approach to the exported functions
It contains an extend flag which indicates the expression
should be copied to the .g file.
This feature is not actually used here, so could be removed.

=cut

{
package decode ;

use Exporter ;
@ISA = ('Exporter') ;
@EXPORT = qw(enum_decode define_decode nametable) ;

*main = *main::main ;

*stem = *main::stem ;
*file = *main::file ;
*pre  = *main::pre ;
*post = *main::post ;

use common ;

use Interpolation
    E => 'eval',
    B => sub {$_->{$_[0]}},
;

import Interpolation
    K => sub {sprintf("%-24s",$_->{$_[0]})},
;


use filecontents ;

$state = {
} ;

=pod
These module immediately create content on evaluation
this is simpler and more flexible.

Other modules create a full list of items and generate code
once all items have been created.

=cut
################################################################
sub c_include {
    my $k = shift ;
    my $f = shift || "$k.h" ;
    if (!$state->{header}->{$k}) {
	push @{$output->{h}},"#include	$f\n\n" ;
	$state->{header}->{$k} = 1 ;
    }
}
	
################################################################
sub glt_out_items {
    my $i = shift ;
    my $o ;
    for (@$i) {
	my ($value,$name) = @$_ ;
	push @$o,"    GLTITEM($value,\"$name\")" ;
    }
    join(",\n",(@$o,"    GLTENDTABLE")) ;
}

sub glt_out {
    my ($tag,$items) = @_ ;
    push @{$output->{c}},(
	"GLT glt_$tag\[] = {\n",
	glt_out_items($items),
	"\n} ;\n\n",
    ) ;
    c_include "sk","\"glt.h\"" ;
    push @{$output->{h}},"extern GLT glt_$tag\[] ;\n",
}


################################################################
sub sk_out_items {
    my $i = shift ;
    my $o ;
    for (@$i) {
	my ($tag,$name) = @$_ ;
	push @$o,"    {$tag,\"$name\"}" ;
    }
    join(",\n",(@$o,"    SKENDTABLE")) ;
}

sub sk_out {
    my ($tag,$items) = @_ ;
    push @{$output->{c}},(
	"SK sk_$tag\[] = {\n",
	sk_out_items($items),
	"\n} ;\n\n",
    ) ;
    c_include "sk","<pu/sk.h>" ;
    push @{$output->{h}},"extern SK sk_$tag\[] ;\n",
}

################################################################
sub stringarray_out {
    my ($tag,$items) = @_ ;
    push @{$output->{c}},(
	"char *names_$tag\[] = {\n",
	(map {"    \"$_->[1]\",\n" } @$items),
	"\n} ;\n\n",
    ) ;
    push @{$output->{h}},"extern char *names_$tag\[] ;\n",
}

################################################################
sub code_text {
    my $a = shift ;
    if (defined $a->{file}) {return file_contents($a->{file}) ;}
    return $main::text ;
}

################################################################
sub decode_out {
    my ($tag,$items,$a) = @_ ;
    my $format = "$a->{format}_out" ;
    if (!defined &{$format}) {
	die "unknown formatter: $a->{format}" ;
	}
    $format = eval "\\&{$format}" ;
    push @{$output->{c}},"/* ~~ $main->{expr} */\n\n" ;
    $format->($tag,$items) ;
}

################################################################
sub enum_decode {
    my $tag = shift ;
    my $a = {
	format => "glt",
	@_
    } ;
    my $t = code_text @_ ;
    $t =~ s!.*enum __enum_$tag\s*{!!s ;
    $t =~ s!}.*!!s ;
    $t =~ s!\n!!gs ;
    $t =~ s!\s*=\s*\w+,!,!gs ;
    $t = trim $t ;
    my $items = [split(m!\s*,\s*!,$t)] ;
    $items = [map {[$_,$_]} @$items] ;
    decode_out "enum_$tag",$items,$a ;
}

################################################################
sub define_decode {
    my $pat = shift ;
    my $a = {
	transform => '"$1"',
	format => "glt",
	@_
    } ;
    my $tag = $a->{tag} ;
    my $transform = $a->{transform} ;
    my $t = code_text $a ;
    if (!$tag) {
	$pat =~ m!^\w+! ;
	$tag = $& ;
    }
    my $items ;
    while ($t =~ m!\#define\s*($pat)\s*(.*)!g) {
	my $s = eval $a->{transform} ;
	push @$items,[$1,$s,$2] ;
    }
    decode_out $tag,$items,$a ;
}

################################################################
sub nametable {
    my $pat = shift ;
    my $a = {
	transform => '"$1"',
	format => "glt",
	@_
    } ;
    my $tag = $a->{tag} ;
    my $transform = $a->{transform} ;
    my $t = code_text $a ;
    if (!$tag) {
	$pat =~ m!^\w+! ;
	$tag = $& ;
    }
    my $items ;
    while ($t =~ m!$pat!gm) {
	my $s = eval $a->{transform} ;
	push @$items,[$1,$s,$2] ;
    }
    decode_out $tag,$items,$a ;
}

################################################################
sub gen_h {
    my $o ;
    push @$o,@{$output->{h}} ;
    @$o ;
}

sub gen_c {
    my $o ;
    push @$o,@{$output->{c}} ;
    @$o ;
}

$module = {
    name => "decode",
} ;

main::module $module->{name} ;

}

1 ;

__END__
