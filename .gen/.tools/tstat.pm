=pod

=cut

{
package tstat ;

use Exporter ;
@ISA = ('Exporter') ;
@EXPORT = qw(tsf sopen sclose) ;

$module = {
    name => "tstat",
} ;

main::module $module->{name} ;

*main = *main::main ;

*stem = *main::stem ;
*file = *main::file ;
*pre  = *main::pre ;
*post = *main::post ;

use Data::Dumper ;

use Interpolation
    E => 'eval',
    B => sub {$_->{$_[0]}},
;

import Interpolation
    K => sub {sprintf("%-24s",$_->{$_[0]})},
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
	name => shift,
	@_
    } ;
#    print "sopen $s->{name}\n" ;
}

################################################################
sub tsf {
    my $f = {
	name => shift,
	type => shift,
	@_,
    } ;
    push @{$s->{fields}},$f ;
}

################################################################
sub start {
}

sub finish {
    sclose ;
#    dump_items $items ;
}

################################################################
sub type_pointer_destar {
    my $t = shift ;
    $t =~ s! !!g ;
    if ($t =~ s!\*!p!g) {return uc($t)}
    return uc($t) ;
}

sub struct_h {
    my $s = shift ;
    my ($tag,$fields) = @{$s}{qw(name fields)} ;
    my $o ;
    my $osf ;		# struct field declaration 
    my $osfi ;		# struct field index number #define
    my $ts ;
    my $sfi = 0 ;
    for (@$fields) {
	push @$osf,sprintf("    %-12s %s ;\n",@{$_}{qw(type name)}) ;
	push @$osfi,sprintf("#define %-32s %d\n","SFI_${tag}_$_->{name}",$sfi++) ;
    }
    push @$osfi,sprintf("\n#define %-32s %d\n","SFI_${tag}__END",$sfi) ;
    push @$o,@$osfi ;
    push @$o,"\nstruct struct_$tag {\n" ;
    push @$o,@$osf ;
    push @$o,"} ;\n\n" ;
    push @$o,"extern char *${tag}_sfnames[] ;\n" ;
    push @$o,"extern SF ${tag}_sf[] ;\n" ;
    push @$o,"\n" ;
    @$o ;
}

sub struct_c {
    my $s = shift ;
    my ($tag,$fields) = @{$s}{qw(name fields)} ;
    my $o ;
    my $ofs ;		# field descriptors
    my $ofn ;		# field name strings
    
    for (@$fields) {
	my $name = $_->{name} ;
	push @$ofn,sprintf("  \"%s\",\n",$name) ;
	push @$ofs,sprintf("  {SFT_%s,\"%s\",SF_O(%s,%s)},\n",
			  type_pointer_destar ($_->{type}),
			  $name,$tag,$name) ;
    }
    push @$o,"\nSF ${tag}_sf[] = {\n" ;
    push @$o,@$ofs ;
    push @$o,"  {SFT__END}\n} ;\n\n" ;

    push @$o,"char *${tag}_sfnames[] = {\n" ;
    push @$o,@$ofn ;
    push @$o,"} ;\n\n" ;

    @$o ;
}

sub gen_h {
    my $o ;
    for $s (@{$items->{s}}) {
	push @$o,"/* $s->{name} */\n\n" ;
	push @$o,struct_h $s ;
    }
    @$o ;
}

sub gen_c {
    my $o ;
    for $s (@{$items->{s}}) {
	push @$o,"/* $s->{name} */\n" ;
	push @$o,struct_c $s ;
    }
    @$o ;
}

}

1 ;

__END__
