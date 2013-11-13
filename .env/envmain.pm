use hostenv ;
use stdmain ;

END {
    $arg->{host} = `hostname` ;
    chomp $arg->{host} ;
}

sub comment_hash {
    "## " . join("",@_) ;
}

sub comment_semi {
    ";; " . join("",@_) ;
}

$comment = \&comment_hash ;

sub ghead {
    "##generated## from <$0>" ;
}

sub gtail {
    my @o ;
    push @o,$comment->("Local\ Variables:\n") ;
    push @o,$comment->("first-change-hook : (not-this-file)\n") ;
    push @o,$comment->("End:\n") ;
    join("",@o) ;
}

use Getopt::Long ;

sub getargd {
    my @a = grep m!\.locate!,@INC ;
    my $locate = $a[0] ;
    my $env = $hostenv->{$arg->{host}} ;

    for (keys %$env) {$arg->{$_} ||= $env->{$_}}

    my @argspec = argspec ;
    Getopt::Long::Configure('no_pass_through') ;
    GetOptions($arg,@argspec) ;

    for (keys %$arg) {
	my $s = "\$$_ = \"$arg->{$_}\"" ;
	eval $s ;
    }
}

sub simple {
    getargd ;
    while (<DATA>) {
	m!__ENDEND__! and last ;
	if (m/!!ghead/) {
	    if (m!^\s*\#!) { $comment = \&comment_hash ;}
	    if (m!^\s*;!) { $comment = \&comment_semi ;}
	    }
	s/!!(.*?)!!/$1/eeg ;
	print ;
    }
    }

$arg = {
    %$arg,
    cmd => "simple",
} ;

1 ;
