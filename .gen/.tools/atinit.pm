=pod

This is an example of a very small module.
Due to overlap with 'collect' features it will get even smaller

atinit is analogous to axexit. It pre-registers functions
that are called during phases of initialization.

=cut

{
package atinit ;

use collect ;

sub register {
    my $function = shift ;
    if (!$function) {
	$function = "${main::stem}_atinit" ;
    }
    collect::register('atinit',$function) ; ;
}

sub main {
    main::gen_add_g "use collect ;\n" ;
}
}

atinit::main() ;
