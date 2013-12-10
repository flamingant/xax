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

sub sym_c_to_lisp ($) {
    my $s = shift ;
    $s =~ tr!_!-! ;
    $s ;
}

sub unadorn ($) {
    my $s = shift ;
    $s =~ s!^[FQVKS]*!! ;
    $s =~ s![FQVKS]*$!! ;
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
    my $o ;
    push @$o,"LTD ${t}_ltd[] = {{\n" ;
    for (@$f) { push @$o,"    $_,\n" ;}
    push @$o,"    }} ;\n\n" ;
    push @{$items->{cout}},@$o ;
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

    $f->{lname} = unadorn sym_c_to_lisp $f->{name} ;

    $rp->{doc} = unquote $rp->{doc} ;

    if (defined($rp->{"name"})) {
	$f->{lname} = unquote $rp->{"name"}
    }
    if (defined($rp->{"l"})) {
	$f->{lisp} = 1 ;
	for (split(/\s+/,$rp->{"l"})) {
	    $f->{"lisp$_"} = 1 ;
	}
    }
    if (defined($rp->{"argc"})) {
	my @a = split(/\s+/,$rp->{"argc"}) ;
	$f->{argcmin} = $a[0] ;
	$f->{argcmax} = $a[1] ;
    }
    else {
	$f->{argcmin} = @{$f->{args}} ;
	$f->{argcmax} = @{$f->{args}} ;
    }

    $f = {%$f,hslice($rp,qw(keys doc trap_exec side_effects allocates command))} ;

    push @{$items->{fun}},$f ;
}

sub find_F {
    my $o ;
    local $_ = $gtext ;
    my $name ;
    while (m!/\*\(F\s*(.*?)\)\s*\*/!gs) {
	my $f = cfparse $' ;
	one_F $f,$1 ;
    }
    for $item (@{$items->{fun}}) {
	if ($item->{lisp}) {
	    my $c ;
	    $c .= "$Q{lname},(lfun) $I{name},subret_$I{type}," ;
	    $c .= "$I{argcmin},$I{argcmax}," ;
	    $c .= "0," ;
	    $c .= "{{$I{trap_exec},$I{side_effects},$I{lispA},$I{command}}}," ;
	    $c .= $item->{lispk} ? "K$I{name}," : "0," ;
	    $c .= "$Q{doc}" ;
	    push @{$items->{lsub}},{c => "{$c}"} ;
	}
	if ($f->{class} eq 'extern') {
	    push @{$items->{hout}},"$f->{dec} ;\n" ;
	}
    }
    if (@{$items->{lsub}}) {
	push @$o,"static lo_sub sub_mod[] = {\n" ;
	for $item (@{$items->{lsub}}) {
	    push @$o,"    $I{c},\n" ;
	}
	push @$o,"    {0}} ;\n\n\n" ;
    }
    push @{$items->{cout}},@$o ;
}

################################################################
sub one_Q {
    my ($name,$props) = @_ ;
    my $rp = lprops $props ;
    my $i = {
	name => $name,
	lname => unadorn sym_c_to_lisp $name,
    } ;
    if (defined($rp->{name})) {
	$i->{lname} = unquote $rp->{name} ;
    }
    push @{$items->{sym}},$i ;
}

sub find_Q {
    my $os ;
    my $o ;
    local $_ = $gtext ;
    my $name ;
    while (m!lo\s+(\w+).*?/\*\(Q\s*(.*?)\)\s*\*/!g) {
	one_Q $1,$2 ;
    }
    $items->{sym} = [reverse @{$items->{sym}}] ;
}

sub out_Q {
    my $os ;
    my $o ;

    if (@{$items->{sym}}) {
	push @$o,"static struct sym_init sym_mod[] = {\n" ;
	for $item (@{$items->{sym}}) {
	    push @$o,"    {&$I{name},$Q{lname}},\n" ;
	    if ($item->{undeclared}) {
		push @$os,"lo $item->{name} ;\n" ;
	    }
	}
	push @$o,"    {0}} ;\n" ;
    }
    push @{$items->{cout}},@$os,"\n" ;
    push @{$items->{cout}},@$o,"\n\n" ;
}

################################################################
sub one_V {
    my ($name,$props) = @_ ;
    my $rp = lprops $props ;
    my $i = {
	name => "Q$name",
	lname => unadorn sym_c_to_lisp $name,
	undeclared => 1} ;
    push @{$items->{sym}},$i ;
}

sub find_V {
    my $os ;
    my $o ;
    local $_ = $gtext ;
    my $name ;
    while (m!lo\s+(\w+).*?/\*\(V\s*(.*?)\)\s*\*/!g) {
#	print " ==== $& ====\n" ;
	one_V $1,$2 ;
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
    find_Q ;
    find_V ;

    out_Q ;
    print "\n\n\n" ;
    for (@{$items->{cout}}) {
	print ;
    }
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
