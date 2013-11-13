=pod

collect is a generic collection method. 
gen_collect('foo','bar') adds 'bar' to the set named 'foo'
at generation (gen.c) time a vector of references to the
collection is created.

=cut

{
package collect ;

sub unregister {
    my $set = shift ;
    my $symbol = shift ;
    main::gen_add_g "gen_collect_remove('$set','$symbol') ;\n" ;
}

sub register {
    my $set = shift ;
    my $symbol = shift ;
    my $extra = shift ;
    $extra ||= "{}" ;
    main::gen_add_g "gen_collect('$set','$symbol',$extra) ;\n" ;
}

sub register_all {
    my $set = shift ;
    my $pat = shift ;
    my $expr = shift ;
    my $t = $main::text ;
    my $f = "while (\$t =~ m!$pat!gm) {register \"$set\",$expr ;}" ;
    eval $f ;
    warn "eval \"$expr\" failed -> $@" if $@ ;
}

sub start {
    main::gen_add_g "use collect ;\n" ;
}
}

collect::start() ;
