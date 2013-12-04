=pod

=cut

{
package mainmode ;

use Exporter ;
@ISA = ('Exporter') ;
@EXPORT = qw(mode) ;

$module = {
    name => "mainmode",
} ;

main::module $module->{name} ;

*main = *main::main ;

*stem = *main::stem ;
*file = *main::file ;
*pre  = *main::pre ;
*post = *main::post ;

use collect ;

use Data::Dumper ;

################################################################
use Interpolation
    'I:@->$' => sub {my $s = $item->{$_[0]} || $_[1] || 0 ; $s},
    'A:@->$' => sub {my $s = $item->{$_[0]} ; "$s\[]" . ' ' x (($_[1] || 20) - length($s))},
    'Q:@->$' => sub {my $s = $item->{$_[0]} ; "\"$s\""},
    'QP:@->$' => sub {my $s = $item->{$_[0]} ; "\"$s\"" . ' ' x (($_[1] || 20) - length($s))},
 ;

################################################################
sub mode {
    my $id = shift || $stem ;

    $item = {name => $id,@_} ;
    $item = {
	desc		=> "$I{name} has no description yet",
	entry		=> "$I{name}_main",
	fun		=> "mmf_null",
	enabled		=> 1,
	%$item} ;

    my $symbol = "mmo_$I{name}" ;
    main::gen_add_g "gen_collect('mainmode','$symbol') ;\n" ;

    push @$reg,$item ;
}

################################################################
sub start {
    main::gen_add_g "use collect ;\n" ;
}

sub finish {
}

################################################################
sub gen_h {
    my $o ;
    push @$o,"/* mainmode { */\n" ;
    push @$o,"\n#include	\"mainmode.h\"\n\n" ;
    for $item (@$reg) {
	push @$o,"extern MMO mmo_$I{name}\[] ;\n" ;
    }
    push @$o,"\n/* mainmode } */\n\n" ;
    @$o ;
}

sub gen_c {
    my $o ;
    push @$o,"/* mainmode { */\n" ;
    for $item (@$reg) {
	push @$o,"MMO mmo_$A{name} = {{
    $I{context},
    $I{entry},
    $I{fun},
    $Q{name},
    $Q{desc},
    $I{default_mmo},
    $I{enabled},
\n}} ;\n\n" ;
    }
    push @$o,"/* mainmode } */\n" ;
    @$o ;
}

}

1 ;

__END__
