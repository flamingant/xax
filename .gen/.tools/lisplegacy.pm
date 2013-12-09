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

use collect ;

use Data::Dumper ;

my $info = {} ;

################################################################
use Interpolation
    'I:@->$' => sub {my $s = $item->{$_[0]} || $_[1] || 0 ; $s},
    'A:@->$' => sub {my $s = $item->{$_[0]} ; "$s\[]" . ' ' x (($_[1] || 20) - length($s))},
    'Q:@->$' => sub {my $s = $item->{$_[0]} ; "\"$s\""},
    'QP:@->$' => sub {my $s = $item->{$_[0]} ; "\"$s\"" . ' ' x (($_[1] || 20) - length($s))},
 ;

################################################################
$tomem = [
    ["fields",	"lo"],
    ["gs_name",	"lo"],
    ["gs_cvx",	"lo"],
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

    push @$f,"\"$t\",\'-\'" ;

    for (qw(gcpro destroy nopointer link number integer symbol string)) {
	if ($props =~ s!$_!!) {
	    push @{$m->{flags}},"tf_$_" ;
	}
    }

    push @$f,join " | ",@{$m->{flags}} ;

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
	else  {
	    push @$f,"tog__$_->[0]" ;
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
sub start {
    if ($text !~ m!cg-start!) { return ;}
#    print "$file is a lisp module\n" ;
    $info->{islisp} = 1 ;
    collect::register('lispmod',"morsel_$stem") ;
    $text =~ m!/\*\(cg-end\).*!s ;
    $gtext = $` ;
    find_T ;
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
