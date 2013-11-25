#!perl

use Data::Dumper ;

use File::Basename ;
use File::Path ;
use Cwd ;

sub gen_add {
    my $s = shift ;
    my $f = shift ;
    my $ss = sprintf($f,@_) ;
    push @{$gen->{$s}},$ss ;
}

# c -> generated module code

sub gen_add_c {
    gen_add "c",@_ ;
}

# h -> generated module header

sub gen_add_h {
    gen_add "h",@_ ;
}

# g -> generic directives

sub gen_add_g {
    gen_add "g",@_ ;
}

# i -> initialization code

sub gen_add_i {
    gen_add "i",@_ ;
}

# mk -> makefile directives

sub gen_add_mk {
    gen_add "mk",@_ ;
}

sub dp {
    my $level = shift ;
    if ($level <= $dbglevel) {
	printf @_ ;
    }
}

################################################################
sub require_all {
    $cwd = getcwd ;
    dp 1,"reading gen.pm in %s\n",getcwd ;
    if (-f "../lgen.pm") {require "../lgen.pm" ;}
    if (-f "./lgen.pm") {require "./lgen.pm" ;}
    chdir ".." ;
    for $o (@object) {
	($stem = $o->[0]) =~ s!\.\w+!! ;
	$file_g = ".gen/$stem.g" ;
	dp 1,"require %s %s\n",$file_g,-f $file_g ? "Y" : "-" ;
	-f $file_g or next ;
	require "$file_g" ;
    }
    chdir $cwd ;
}

sub object {
    my $name = shift ;
    dp 1,"adding object %s\n",$name ;
    push @object,[$name,{@_}] ;
}

sub macro {
    my $name = shift ;
    dp 1,"adding macro %s\n",$name ;
    push @macros,[$name,join(" ",@_)] ;
}

################################################################
=pod
include .gen/{module}.g files for all object files given in gen.pm
all gen material with 'i' tag is evalled and written to stdout
This is usually called from the makefile to create gen.c
=cut

sub init_c {
    my $phase = shift ;
    for $fun (@{genfuns->{$phase}}) {
	$fun->() ;
    }
}

sub init_version {
    my $gf = "genver.pm" ;
    if (-f $gf) {
	require "./$gf" ;
    }
    else {
	$version = {build => 0}
    }

    $version->{build}++ ;
    printf "version = {build => %d}\n",$version->{build} ;

    gen_add_i("#include \"../version.h\"\n") ;
    gen_add_i("VERSION version = {%d} ;\n",$version->{build}) ;
    open V,">$gf" ;
    printf V "\$version = {build => %d} ;\n",$version->{build} ;
    close V ;
}

sub dump_items {
    my $dump = Data::Dumper->new([$allitems]) ;
    my $i = $dump->Terse(1)->Indent(1)->Dump ; ;
    print $i ;
}

sub init {
    $out = shift ;
    push @INC,".gen/.tools/init",".gen/.tools/stub", ;
    if ($out) {open O,">$out" ;} else {*O = *STDOUT ;}
    require_all ;
#    dump_items ;
    init_version ;
    init_c("end") ;
    for (@{$gen->{"i"}}) {print O $_,"\n"}
}

################################################################
=pod
=cut

sub make {
    $out = shift ;
    if ($out) {open O,">$out" ;} else {*O = *STDOUT ;}
    push @INC,".gen/.tools/make",".gen/.tools/stub", ;
    require_all ;
    chdir ".." ;
    for $o (@object) {
	($stem = $o->[0]) =~ s!\.\w+!! ;
	$file_g = ".gen/$stem.g" ;
	-f $file_g or next ;
	require "$file_g" ;
    }

    for (@macros) {
	my ($name,$value) = @$_ ;
	gen_add_mk "$name := $value\n" ;
    }

    for my $g (@linkgroups) {
	gen_add_mk "$g := \\" ;
	for $o (@object) {
	    if ($o->[1]->{linkgroup} eq $g) {
		($stem = $o->[0]) =~ s!\.\w+!! ;
		gen_add_mk "\t\t\%s.o \\",$stem ;
	    }
	}
    gen_add_mk "\t\t\n" ;
    }

    gen_add_mk "all:\t\t%s",join(" ",@targets) ;

    for (@{$gen->{"mk"}}) {print O $_,"\n"}

}

################################################################
sub keytxt {
    push @INC,".gen/.tools/doc",".gen/.tools/stub", ;
    $package = shift || "keys" ;
    require_all ;
}

################################################################
sub install {
    require_all ;
}

################################################################

################################################################
END {my $f = shift @ARGV || "make" ; $f = eval "\\&main::$f" ; $f->(@ARGV) ;}
