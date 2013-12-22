=pod

This is one of the simpler modules, as
.) it doesn't allow multiple sets 
.) and sets are always named after the module
.) nothing is output to .h file
.) no initialized structures are created

It could be upgraded to parse the pre and post to extract
default values like arg does

=cut

{
package http ;

use Exporter ;
@ISA = ('Exporter') ;
@EXPORT = qw(hpf) ;

*main = *main::main ;

*stem = *main::stem ;
*file = *main::file ;
*pre  = *main::pre ;
*post = *main::post ;

use common ;
use collect ;

################################################################
use Interpolation
    'I:@->$' => sub {my $s = $_->{$_[0]} || $_[1] || 0 ; $s},
    'IV:@->$' => sub {my $s = $_->{$_[0]} ; join(" | ",@$s,0) ;},
    'A:@->$' => sub {my $s = $_->{$_[0]} ; "$s\[]" . ' ' x (($_[1] || 20) - length($s))},
    'Q:@->$' => sub {my $s = $_->{$_[0]} ; "\"$s\""},
    'QP:@->$' => sub {my $s = $_->{$_[0]} ; "\"$s\"" . ' ' x (($_[1] || 20) - length($s))},
 ;

################################################################
sub strip_hpf($) {
    local $_ = shift ;
    m!hpf_! ;
    $' ;
    }

sub hpf {
    my $cf = cfparse $post ;

    $item = {
	cname => strip_hpf $cf->{name},
    } ;

    $item = {
	%$item,
	uname => hyphenate $item->{cname},
	@_,
    } ;

    if (!@{$items->{p}}) {
	collect::register('hpoa',"hpoa_${stem}") ;
    }

    push @{$items->{p}},$item ;
}

################################################################
sub start {
}

sub finish {
#    print dump_items($items),"\n" ;
}

################################################################
sub gen_h {
    my $o ;
    @$o ;
}

sub gen_c {
    my $o,$oo ;
    my $tf ;
    local $_ ;
    for $_ (@{$items->{p}}) {
	push @{$oo->{b}},sprintf("    hpo_%s,\n",@{$_}{qw(cname)}) ;

	push @{$oo->{c}},"HPO hpo_$I{cname}\[] = {
    hpf_$I{cname},
    $Q{uname},
    $Q{desc},
    $Q{help},
    } ;\n\n" ;
    }
    push @$o,@{$oo->{c}} ;

    push @$o,"HPO *hpoa_${stem}\[] = {\n" ;
    push @$o,@{$oo->{b}} ;
    push @$o,"    0} ;\n\n" ;

    push @$o,"/* */\n" ;
    @$o ;
}

$module = {
    name => "http",
} ;

main::module $module->{name} ;

}

1 ;

__END__
