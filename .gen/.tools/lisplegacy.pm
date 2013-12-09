=pod

copied from mainmode

=cut

{
package lisplegacy ;

use Exporter ;
@ISA = ('Exporter') ;
@EXPORT = qw(mode) ;

$module = {
    name => "lisplegacy",
} ;

main::module $module->{name} ;

*main = *main::main ;

*stem = *main::stem ;
*file = *main::file ;
*pre  = *main::pre ;
*post = *main::post ;
*text = *main::text ;

use common ;
use collect ;

use Data::Dumper ;

my $info = {} ;

sub unquote {
    my $s = shift ;
    if ($s =~ s!^\"!!) {$s =~ s!\"$!!;}
    $s ;
}

sub csafe {
    my $s = shift ;
    $s =~ tr!-!_! ;
    $s ;
}

sub sym_c_to_lisp {
    my $s = shift ;
    $s =~ tr!_!-! ;
    $s ;
}

sub unadorn {
    my $s = shift ;
    $s =~ s!^[FQ]!! ;
    $s =~ s![FQ]$!! ;
    $s ;
}

################################################################
use Interpolation
    'I:@->$' => sub {my $s = $item->{$_[0]} || $_[1] || 0 ; $s},
    'A:@->$' => sub {my $s = $item->{$_[0]} ; "$s\[]" . ' ' x (($_[1] || 20) - length($s))},
    'Q:@->$' => sub {my $s = $item->{$_[0]} ; "\"$s\""},
    'QP:@->$' => sub {my $s = $item->{$_[0]} ; "\"$s\"" . ' ' x (($_[1] || 20) - length($s))},
 ;

################################################################
$tomem = [
    ["fields",	"lo", "0"],
    ["gs_name",	"lo", "gs__none"],
    ["gs_cvx",	"lo", "gs__none"],
    ["gp",	"lo"],
    ["funcall",	"lo"],
    ["destroy",	"void"],
    ["claim",	"void"],
    ["lread",	"lo"],
    ["eval",	"lo"],
    ["print",	"void"],
    ["format",	"lo"],
    ["copy",	"lo"],
    ["eq",	"lo"],
    ["equal",	"lo"],
    ["length",	"lo_int"],
    ["pplist",	"lo*"],
    ["aref",	"lo"],
    ["aset",	"lo"],
    ["sf_get",	"int"],
    ["sf_set",	"lo"],
    ["symget",	"lo"],
    ["symset",	"lo"],
    ["init",	"void"],
    ["ox_int",	"void"],
    ["ox_str",	"void"],
    ["cmp",	"int"],
    ["pprel",	"void"],
    ["mread",	"lo"],
    ["mwrite",	"lo"],
    ["mdelete",	"lo"],
    ["create",	"lo"],
    ["next",	"lo"],
    ["prev",	"lo"],
    ["this",	"lo"],
    ] ;

sub one_T {
    my $t = shift ;
    my $props = shift ;
    my $m = {} ;
    my $f ;

    $m->{tag} = '-' ;
    if ($props =~ s!\(tag\s*\\*(.)!!) {$m->{tag} = $1 ;}

    push @$f,"\"$t\",\'$m->{tag}\'" ;

    for (qw(gcpro destroy nopointer link number integer symbol string)) {
	if ($props =~ s!$_!!) {
	    push @{$m->{flags}},"tf_$_" ;
	}
    }

    push @{$m->{flags}},0 unless push @{$m->{flags}} ;

    push @$f,join " | ",@{$m->{flags}} ;

    if ($props =~ m!\(alloc\s*(.*?)\)!) {
	my @a = split(/\s+/,$1) ;
	if ($a[0] eq 'f') {
	    push @$f,"{0,nnc_nil, $a[1],sizeof(lo_$t),0,  0,0,0}" ;
	}
	elsif ($a[0] eq 'v') {
	    push @$f,"{0,nnc_nil, $a[1],sizeof(lo_$t),0,  $a[4],$a[3],$a[2]}" ;
	}
    }
    else {
	push @$f,"{0}" ;
    }

    local $_ = $gtext ;
    my $pat ;
    $pat = join("|",map $_->[0],@$tomem) ;
    $pat = "\\b${t}__($pat)\\b" ;
    while (m!$pat!g) {
	$m->{$1} = 1 ;
    }
    for (@$tomem) {
	if ($m->{$_->[0]}) {
	    push @$f,"${t}__$_->[0]" ;
	}
	else {
	    if (@$_ > 2) {
		push @$f,$_->[2] ;
	    }
	    else {
		push @$f,"tog__$_->[0]" ;
	    }
	}
    }
    print "LTD ${t}_ltd[] = {{\n" ;
    for (@$f) { print "    $_,\n" ;}
    print "    }} ;\n\n" ;
}

sub find_T {
    local $_ = $gtext ;
    while (m!/\*\(T\s*(\w+)(.*?)\)\*/!g) {
	my $t = $1 ;
	one_T $1,$2 ;
	   }
}

################################################################
sub one_F {
    my $f = shift ;
    my $props = shift ;
    my $rp = lprops $props ;
#    print "$props\n" ;
#    print join("|",keys %$rp),"\n" ;

    $f->{lname} = $f->{name} ;
    if (defined($rp->{"name"})) {
	$f->{lname} = unquote $rp->{"name"}
    }
    if (defined($rp->{"l"})) {
	$f->{lisp} = 1 ;
    }

    push @{$items->{fun}},$f ;
}

sub find_F {
    local $_ = $gtext ;
    my $name ;
    while (m!/\*\(F\s*(.*?)\)\*/!g) {
	my $f = cfparse $' ;
	one_F $f,$1 ;
    }
    for $item (@{$items->{fun}}) {
	if ($item->{lisp}) {
	    my $c ;
	    $c = "{$Q{lname},(lfun) $I{name}}" ;
	    push @{$items->{lsub}},{c => $c} ;
	}
	if ($f->{class} eq 'extern') {
#	    print "$f->{dec} ;\n" ;
	}
    }
    for $item (@{$items->{lsub}}) {
	print "    $I{c},\n" ;
    }
}

################################################################
sub start {
    if ($text !~ m!cg-start!) { return ;}
#    print "$file is a lisp module\n" ;
    $info->{islisp} = 1 ;
    collect::register('lispmod',"morsel_$stem") ;
    $text =~ m!/\*\(cg-end\).*!s ;
    $gtext = $` ;
    find_T ;
    find_F ;
}

sub finish {
}

################################################################
sub gen_h {
    my $o ;
    return unless $info->{islisp} ;
    push @$o,"/* $module->{name} { */\n" ;
    push @$o,"\n/* $module->{name} } */\n\n" ;
    @$o ;
}

sub gen_c {
    my $o ;
    return unless $info->{islisp} ;
    push @$o,"/* $module->{name} { */\n" ;
    push @$o,"/* $module->{name} } */\n" ;
    @$o ;
}

}

1 ;

__END__
