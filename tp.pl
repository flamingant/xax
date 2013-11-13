#!perl -I.. -Ie:/perl/lib -Ie:/perl/lib/fly

$| = 1 ;

use stdmain ;

use LWP::UserAgent;
use HTTP::Request;
use HTTP::Cookies;

use Data::Dumper ;

$arg = {
    BEGIN	=> "begin",
} ;

sub begin {
    $ua = new LWP::UserAgent ;
}

################################################################
use Socket;

sub get {
    my $method = "GET" ;
    my $uri = shift ;
    my $request = new HTTP::Request($method,$uri) ;
#    print Dumper $request ;
    my $response = $ua->request($request) ;
#    print Dumper $response ;
    $response->{_content} ;
}

sub tget {
    my $id = shift ;
    my $url = shift ;
    my $d = sprintf("e:/_/T/torrent/.torrent/%d00",$id / 100) ;
    my $f = "$d/$id.torrent" ;
    mkdir $d unless -d $d ;
    if (-f $f) {
	return file_contents $f ;
    }
    my $url = "https://what.cd/torrents.php?action=download&id=$id&authkey=4811b35a4613a8d8e32924e371929154&torrent_pass=lh6umeb41zsccsi7xdizmedvjinubbia" ;
    my $s = get $url ;
    open O,">$f" ;
    binmode O ;
    print O $s ;
    close O ;
    $s ;
}

sub main {
    $server_port = 5008 ; 

    socket(SERVER, PF_INET, SOCK_STREAM, getprotobyname('tcp'));

    setsockopt(SERVER, SOL_SOCKET, SO_REUSEADDR, 1);

    $my_addr = sockaddr_in($server_port, INADDR_ANY);

    bind(SERVER, $my_addr)
	or die "Couldn't bind to port $server_port : $!\n";

    listen(SERVER, SOMAXCONN)
	or die "Couldn't listen on port $server_port : $!\n";

    print "listening on port $server_port\n";

    while (accept(CLIENT, SERVER)) {
	my $s = "" ;
	while (<CLIENT>) {
	    last if m!^$! ;
	    $s .= $_ ;
	}
	if ($s =~ m!GET (\d+)!) {
	    print "done $s\n" ;
	    print CLIENT "done $s\n\n" ;
	    close CLIENT ;
	    tget $1 ;
	}
    }

close(SERVER);
}
