=pod

=cut

sub gen_collect {
    my $set = shift ;
    my $symbol = shift ;
    push @{$allitems->{collect}->{$set}},$symbol ;
}

sub gen_collect_remove {
    my $set = shift ;
    my $symbol = shift ;
    $allitems->{collect}->{$set} =
	[grep {$_ ne $symbol} @{$allitems->{collect}->{$set}}] ;
}

################################################################
# create initialization vector

sub one_vec {
    my $set = shift ;
    my $items = shift ;
    my $t = <<'.' ;
!!$items_extern!!
int *!!$set!!_initvec[] = 
{
!!$items_vec!!
    0
} ;
.
    my $items_extern = join("",map ("extern int $_\[] ;\n",@$items)) ;
my $items_vec = join("\n",map ("    $_,",@$items)) ;
$t =~ s/!!(.*?)!!/$1/eeg ;
gen_add('i',$t) ;
}

sub collect_gen_c_out {
    my $items = $allitems->{collect} ;
    my @sets = keys %$items ;
    for $set (@sets) {
	one_vec $set,$allitems->{collect}->{$set} ;
    }
}

push @{genfuns->{end}},\&collect_gen_c_out ;
1 ;
