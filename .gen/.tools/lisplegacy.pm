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

$info = {} ;

################################################################
use Interpolation
    'E' => 'eval',
    'I:@->$' => sub {my $s = $item->{$_[0]} || $_[1] || 0 ; $s},
    'A:@->$' => sub {my $s = $item->{$_[0]} ; "$s\[]" . ' ' x (($_[1] || 20) - length($s))},
    'Q:@->$' => sub {my $s = $item->{$_[0]} ; "\"$s\""},
    'QP:@->$' => sub {my $s = $item->{$_[0]} ; "\"$s\"" . ' ' x (($_[1] || 20) - length($s))},
 ;

sub iexpand {
    local $_ = shift ;
    my $v = {@_} ;
    my $item = $v->{item} ;
    my $o ;
    while (!$done) {
	$done = 1 ;
	$o = '' ;
	while (m/!!(.*?)!!(\n|)/m) {
	    my $e = eval($1) ;
	    $done = 0 ;
	    $o .= $` . $e ;
	    $_ = $' ;
	}
	$o .= $_ ;
	$_ = $o ;
    }
    $o ;
}

################################################################
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
    push @{$items->{ltd}},$t ;
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
	$f->{lname} = unquote $rp->{"name"} ;
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
	if ($item->{lisps}) {
	    push @{$items->{sym}},{
		name => "Q$item->{name}",
		lname => $item->{lname},
		undeclared => 1} ;
	}
    }
}

sub out_F {
    my $o ;
    if (@{$items->{lsub}}) {
	push @$o,"static lo_sub sub_mod[] = {\n" ;
	for $item (@{$items->{lsub}}) {
	    push @$o,"    $I{c},\n" ;
	}
	push @$o,"    {0}} ;\n\n" ;
    }
    push @{$items->{cout}},@$o ;
}

################################################################
sub one_x {
    my ($iname,$name,$props) = @_ ;
    my $rp = lprops $props ;
    my $i = {
	name => $name,
	lname => unadorn sym_c_to_lisp $name,
    } ;
    if (defined($rp->{name})) {
	$i->{lname} = unquote $rp->{name} ;
    }
    push @{$items->{$iname}},$i ;
}

sub find_x {
    my ($k,$iname) = @_ ;
    my $os ;
    my $o ;
    local $_ = $gtext ;
    my $name ;
    while (m!lo\s+(\w+).*?/\*\($k\s*(.*?)\)\s*\*/!g) {
	one_x $iname,$1,$2 ;
    }
    $items->{$iname} = [reverse @{$items->{$iname}}] ;
}

sub out_x {
    my $it = shift ;
    my $name = shift ;
    my $os ;
    my $o ;

    if (@$it) {
	push @$o,"static struct sym_init ${name}_mod[] = {\n" ;
	for $item (@$it) {
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
sub find_Q {
    find_x "Q","sym" ;
}
    
sub out_Q {
    out_x $items->{sym},"sym" ;
}

################################################################
sub find_K {
    find_x "K","keyword" ;
}

sub out_K {
    out_x $items->{keyword},"key" ;
}

################################################################
sub find_S {
    find_x "S","usym" ;
}

sub out_S {
    out_x $items->{usym},"tsym" ;
}

################################################################
sub one_V {
    my ($name,$props) = @_ ;
    my $rp = lprops $props ;
    if ($rp->{init}) {$rp->{init} = eval $rp->{init};}
    my $i = {
	vname => $name,
	name => "Q$name",
	lname => unadorn sym_c_to_lisp $name,
	undeclared => 1,
	init => "Qnil",
	%$rp,
    } ;
    push @{$items->{sym}},$i ;
    push @{$items->{var}},$i ;
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
sub mod_toinit {
    @{$items->{ltd}} or return "" ;
    my $t = '    case MOD_TOINIT:
!!join "",map {"\tltd_static_register(${_}_ltd) ;\n"} @{$items->{ltd}}!!
	break ;
' ;
}

sub mod_subinit {
    my $t = '    case MOD_SUBINIT:
!!if (@{$items->{lsub}}) {"\tdefsubs(sub_mod) ;\n"}!!
	break ;
' ;
    $t ;
}

sub mod_syminit {
    my $t = '    case MOD_SYMINIT:
!!if (@{$items->{sym}}) {"\tdefsyms(sym_mod) ;\n"} else {""}!!
!!if (@{$items->{keyword}}) {"\tdefkeys(key_mod) ;\n"} else {""}!!
!!if (@{$items->{usym}}) {"\tdeftsyms(tsym_mod) ;\n"} else {""}!!
	break ;
' ;
    $t ;
}

sub mod_varinit {
    my $v ;
    for $item (@{$items->{var}}) {
	push @$v,"\tdefvar_lo($I{name},&$I{vname},$I{init}) ;\n" ;
    }
    join("",
	 "    case MOD_VARINIT:\n",
	 @$v,
	 "\tbreak ;\n"
	) ;
}

sub out_mimf {
    my $o ;
    local $item = {
	sym_mod => @{$items->{sym}} ? "sym_mod" : "0",
	sub_mod => @{$items->{lsub}} ? "sub_mod" : "0",
    } ;
    my $t = <<'_' ;
static int mod_mimf(int level)
{
    int r = 0 ;
    switch(level) {
!!mod_toinit!!
!!mod_subinit!!
!!mod_syminit!!
!!mod_varinit!!
    }
    return(r) ;
    }

struct mod mod_!!$stem!![] = {{"!!$stem!!",mod_mimf,!!"$I{sym_mod}"!!,!!"$I{sub_mod}"!!}} ;

static int morsel_fun(int i,void *a)
{
    switch(i) {
    case MOM_LISPSTART: lisp_mod_add(mod_!!$stem!!) ; break ;
}
    return(0) ;
}

MORSEL morsel_!!$stem!![] = {morsel_fun} ;

_
## ~~~~~~~~~~~~~~~~
    push @$o,iexpand $t ;
    push @{$items->{cout}},@$o,"\n\n" ;
}


################################################################
sub start {
    if ($text !~ m!cg-start!) { return ;}
    $info->{islisp} = 1 ;
    collect::register('lispmod',"morsel_$stem") ;
    $text =~ m!/\*\(cg-end\).*!s ;
    $gtext = $` ;
    find_T ;
    find_F ;
    find_Q ;
    find_V ;
    find_K ;
    find_S ;

    out_F ;
    out_Q ;
    out_K ;
    out_S ;

    out_mimf ;

    print "\n\n" ;
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
