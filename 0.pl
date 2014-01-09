#!perl -Ie:/perl/lib

$| = 1 ;

use stdmain ;

$arg = {%$arg,
} ;

sub shell {
    my $tmp = "$$.tmp" ;
    open O,">$tmp" ;
    print O @_ ;
    close O ;
    system "bash $tmp" ;
    unlink $tmp ;
}

sub _1222_1028 {
#    shell "git remote rm buu" ;
    shell "git remote rm tole" ;
    
}

sub _1222_1027 {
    shell "git remote add smarf e:/.p/f/d" ;
}

sub _1222_1027 {
    shell "git push -n -v smarf" ;
}

sub _1222_1101 { shell "git push -n -v smarf HEAD" }

sub _1222_1206 { shell "git push -n -v smarf lib" }

sub _1222_1359 { shell "git remote rm smarf" }   # moving to better location

__END__

=pod
remote smarf is on branch ant. We need to fast forward ant to current work
because we can only (?) push back the branches that exist in the remote
repository. If we have created new branches that don't exist in the remote
what happens if we try to push back branches that don't exist?

It doesn't matter what the current branch is

this would be more straightforward if we only had one remote 
=cut
