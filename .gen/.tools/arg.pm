=pod

=cut

{
package arg ;

use Exporter ;
@ISA = ('Exporter') ;
@EXPORT = qw(arg argset) ;

*main = *main::main ;

*stem = *main::stem ;
*file = *main::file ;
*pre  = *main::pre ;
*post = *main::post ;

use common ;
use collect ;

use Data::Dumper ;

################################################################
use Interpolation
    'I:@->$' => sub {my $s = $item->{$_[0]} || $_[1] || 0 ; $s},
    'IV:@->$' => sub {my $s = $item->{$_[0]} ; join(" | ",@$s,0) ;},
    'A:@->$' => sub {my $s = $item->{$_[0]} ; "$s\[]" . ' ' x (($_[1] || 20) - length($s))},
    'Q:@->$' => sub {my $s = $item->{$_[0]} ; "\"$s\""},
    'QP:@->$' => sub {my $s = $item->{$_[0]} ; "\"$s\"" . ' ' x (($_[1] || 20) - length($s))},
 ;

################################################################
sub sclose {
    return unless $s ;
    push @{$items->{s}},$s ;
#    print "sclose $s->{name}\n" ;
    $s = {} ;
}

sub sopen {
    sclose ;
    $s = {
	omf	=> "argset_omf_null",
	name	=> $stem,
	@_
    } ;
#    print "sopen $s->{name}\n" ;
    collect::register('argset',"argset_$s->{name}",$i) ;
}

################################################################
sub arg_add {
    my $o = shift ;
    push @{$s->{args}},$o ;
}

sub fun_arg_parse {
    my $a ;
    my $cf = cfparse $post ;
#    quickdump $cf ;
    my $body = $cf->{body} ;

    local $_ = $body ;
    m!return\s*\((.*)\)! and push @{$a->{flags}},$1 ;

    my $pa ;
    if ($body =~ m!~\+\((.*?)\)\+~!g) {
	$pa = eval "{$1}" ;
    }

    $cf->{name} =~ m!(\w+)__(\w+)! ;

    $a->{accept} = $cf->{name} ;
    $a->{cname} = $2 ;
    {%$a,%$pa} ;
}

sub arg {
    my $a = {
	omf	=> "argitem_omf_null",
	type	=> "char",
	matcher => "(int (*)(char *,char *)) strcmp",
	@_
    } ;

    if (!$a->{inline}) {
	$a = {%$a,fun_arg_parse} ;
    }

    if (defined($a->{argname}) && !defined($a->{cname})) {
	($a->{cname} = $a->{argname}) =~ tr!-!_! ;
    }
    if (defined($a->{cname}) && !defined($a->{argname})) {
	($a->{argname} = $a->{cname}) =~ tr!_!-! ;
    }
    $a->{key} = $a->{cname} ;

    if ($a->{short}) {
	push @{$a->{flags}},"ASF_ARGNAMESHORT" ;
    }
    arg_add {%$a,%$pa} ;
}

sub argset {
    sopen @_ ;
}

################################################################
sub start {
}

sub finish {
    sclose ;
#    dump_items $items ;
}

################################################################
sub argset_h {
    my $o ;
    local $item = shift ;
    push @$o,"extern ARGITEM *args_$I{name}\[] ;\n" ;
    push @$o,"extern ARGSET argset_$I{name}\[] ;\n" ;
    @$o ;
}

sub argset_c {
    my $set = shift ;
    my $o ;
    my $os ;
    my $tf ;
    push @$o,"ARGITEM *args_$set->{name}\[] = {\n" ;
    for (@{$set->{args}}) {
	$item = {%$_,
		 type => uc("AIT_$_->{type}"),
	} ;
	push @$os,"static ARGITEM arg__$A{cname} = {{
    $Q{help},
    $Q{argname},
    $Q{value},
    $I{matcher},
    $I{accept},
    $I{type},
    $IV{flags},
    $I{omf},
}} ;\n\n" ;
	push @$o,sprintf("    arg__%s,\n",@{$item}{qw(cname)}) ;
    }
    push @$o,"    0} ;\n\n" ;

    $item = $set ;
    push @$o,"ARGSET argset_$I{name}\[] = {
	args_$I{name},
	$Q{help},
	$IV{flags},
	$I{omf},
} ;\n

GSV_ARGSET_ADD(argset_$I{name}) ;
" ;
    push @$o,"\n/* ================================================================ */\n" ;
    (@$os,@$o) ;
}

sub gen_h {
    my $o ;
    for (@{$items->{s}}) {push @$o, argset_h $_ ;}
    @$o ;
}

sub gen_c {
    my $o ;
    for (@{$items->{s}}) {push @$o, argset_c $_ ;}
    @$o ;
}

$module = {
    name => "arg",
} ;

main::module $module->{name} ;

}

1 ;

__END__
