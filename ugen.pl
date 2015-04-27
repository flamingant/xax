#!perl -I.gen/.tools -I.gen/.tools/.lib 

use File::Basename ;
use File::Path ;
use Cwd ;

use common ;
use filecontents ;

use open OUT => ":crlf" ;

=pod
my $stem ;
my $path ;
my $lines ;
my $line ;
my $line_number ;
=cut

sub dp {
    my $level = shift ;
    if ($dplevel >= $level) {
	printf @_ ;
    }
}

################################################################
sub gen_add {
    my $s = shift ;
    push @{$gen->{$s}},@_ ;
}

sub gen_add_c {
#    dp 0,"!c! %s !c!",join("",@_) ;
    gen_add "c",@_ ;
}

sub gen_add_h {
    gen_add "h",@_ ;
}

sub gen_add_g {
    gen_add "g",@_ ;
}

sub gen_flush {
    for $s (sort keys %$gen) {
	my $c = join("",@{$gen->{$s}}) ;
	my $ss = $s ;
	$ss =~ s!:.*!! ;
	my $f = ".gen/$stem.$ss" ;
#	dp 0,"$f\n" ;
	if ($c || -f $f) {
	    unlink $f if $ss eq $s ;
	    open O,">>$f" or die "cannot open '$f'" ;
	    print O $c ;
	    close O ;
	}
    }
    $gen = {} ;
}

################################################################
sub evalgen {
    my $t = $text ;
    while ($t =~ m/~~\s*(.*?)\s*~~/gs) {
	my $expr = $1 ;
	$pre = $` ;
	$post = $' ;
	$main->{expr} = $expr ;
	my $ee = eval $main->{expr} ;
	warn "eval \"$main->{expr}\"\n  failed -> $@" if $@ ;
    }
}

sub funcallsafe {
    my $mod = shift ;
    my $e = shift ;
    $e =~ m!.*?(?=\()! ;
    my $f = "$mod" . "::$&" ;
    if (defined &{$f}) {
	$e = "$mod" . "::$e" ;
#	dp 0,"$e\n" ;
	@r = eval $e ;
	warn "eval \"$e\"\n  failed -> $@" if $@ ;
	@r ;
    }
    else {
    }
}

################################################################
sub gen_one {
    my $t = $text ;
    while ($t =~ m!~#\s*(.*?)\s*#~!msg) {
	$expr = $1 ;
        $expr =~ m!^{! and $expr = "do $expr" ;
	eval $expr ;
	print "$expr -> $@\n" if $@ ;
    }
    my $mod ;
#    for $mod (@modules) { quickdump eval "\$$mod" . "::module" ; }

    for $mod (@modules) { funcallsafe $mod,"start()" ; }
    evalgen ;
    for $mod (@modules) { funcallsafe $mod,"finish()" ; }
    for $mod (@modules) {
	gen_add_h funcallsafe $mod,"gen_h()" ;
	gen_add_c funcallsafe $mod,"gen_c()" ;
	gen_add_g funcallsafe $mod,"gen_g()" ;
    }
}

sub get_source {
    my $t ;
    for (qw(h)) {
	if (-f "$stem.$_") {
	    $t .= file_contents("$stem.$_",1) ;
	}
    }
    $t .= file_contents($file,1) ;
    $t ;
}

sub gen {
    $gen = {} ;
    $cwd = getcwd ;
    for $file (@_) {
	$file =~ s!(.*)/!! ;
	$path = $1 ;
	chdir $path if $path ;
	next unless -d ".gen" ;
	($stem = $file) =~ s!\.(\w+)!! ;
	$file_g = ".gen/$stem.g" ;
	$text = get_source ;
	gen_one() ;
	gen_add_g ("1 ;\n\n__END__\n") ;
	gen_flush() ;
	chdir $cwd ;
    }
}

use feature 'state' ;

sub module {
    state $m = {} ;
    my $name = shift ;
    if (!$m->{$name}) {
	$m->{$name} = 1 ;
	push @modules,$name ;
    }
}


END {$command = shift @ARGV || "gen" ; $f = eval "\\&main::$command" ; $f->(@ARGV) ;}

=pod
gen directive blocks can span multiple lines
~= ... =~	for immediate execution 
~~ ... ~~	execution in evalgen loop
~+ ... +=	added attributes in local context

what has been gained in the new regime
. simpler
. more consistent
. no artificial syntax
. any perl evaluable can go between the ~~ ... ~~ (possible disadvantage)
. we get better error detection (replace unimplemented directives with stubs)

and lost
. there is no grep pattern to match a complete gen directive
  however we're not forced to place multiple statements in one directive block
. we have lost the 'keyword' concept, replaced with explicit function name
. no option to just ignore unimplemented directives

=gen

=pod
In order to see what has been added we need to scan the source file
to the end or where we see __gen_end__ which is an agreed marker to say
there is no more __gen__ material.

in emacs the directives can be declarative, e.g markers to say
look for a pattern around this point and derive the appropriate
code from the results of that pattern match.

perl doesn't offer that kind of ability. There isn't the same concept
of a point moving around a buffer extracting text from different locations

In perl the directives need to be triggered by specific patterns

there is no way of avoiding the regeneration if the master file
has been updated.

The .gen file is just a stub to indicate all the gen files are up to date

The gen.c file is compiled if any of the source files change. This is
because any global info may have been added, and the central coordination
of that info (e.g initialization function vectors) may have to be 
rebuilt.

The .g files contain directives that are created by the module gen code
Then executed before the link.

=cut
