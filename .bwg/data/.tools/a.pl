#!perl -I.. -Ie:/perl/lib -Ie:/perl/lib/fly

use stdmain ;

use DBI ;
use Data::Dumper ;

use dbi::dbibase ;

$arg = {
    BEGIN	=> "begin",
} ;

sub begin {
    chdir ".." ;
    $dbh = DBI->connect("DBI:SQLite:dbname=tc.db","","",{AutoCommit => 0}) or die "Can't connect!" ;
}

sub commit {
    $dbh->do("commit") ;
}

################################################################
sub collect {
    my ($file,$tag) = @_ ;
    my @cols ;
    open I,$file ;
sub x {
    my ($name,$type) = @_ ;
    if (0) {}
    elsif ($type =~ m!char\s*\*!) {$type = "text";}
    elsif ($type =~ m!int!) {$type = "integer";}
    elsif ($type =~ m!(float|double)!) {$type = "real";}
    elsif ($type =~ m!enum!i) {$type = "integerl";}
    elsif (0) {}
    "$name $type" ;
}
    while (<I>) {
	m!~~$tag~~(.*)~~! or next ;
	my $x = eval "x$1" ;
	push @cols,$x ;
    }
join(",\n",@cols) ;
}

sub tstatus_create {
    $otherfields = collect("../../ma.c","tsf") ;
    $dbh->do("drop table if exists tstatus") or die ;
    my $cmd = "create table tstatus (
id integer primary key,
hash text primary key,
$otherfields
)" ;
    print "$cmd\n" ;
    $dbh->do($cmd) or die ;
    $dbh->do("commit") ;
}

sub astatus_create {
    $dbh->do("drop table if exists tstatus") or die ;
    my $cmd = "create table tstatus (
id integer primary key,
hash text primary key,
$otherfields
)" ;
    print "$cmd\n" ;
    $dbh->do($cmd) or die ;
    $dbh->do("commit") ;
}

################################################################
sub _0509_1906 {
    gselect $dbh,"select * from tannounce" ;
    
}
sub _0511_1055 {
#    gselect $dbh,"select * from tstatus order by time_added limit 100" ;
    gselect $dbh,"select time_added,name from tstatus order by time_added" ;
}

sub _0511_1149 {
    gselect $dbh,"select time_added from tstatus where hash = '7e85e5c30795cbb53751d3f660aa60056ece9a75'" ;
}

sub _0812_1939 {
    gselect $dbh,"select count(*) from tstatus" ;
}

sub _0812_1939 {
#    gselect $dbh,"select hash,name from tstatus order by hash" ;
    gselect $dbh,"select hash from f" ;
    gselect $dbh,"select hash from ff" ;
}

################################################################
sub main {
}
