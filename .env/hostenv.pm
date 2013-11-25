################################################################
$base = {
    devroot	=> '$drive/.p/.master',
    sqlite	=> '$devroot/sqlite',
} ;

################################################################
$os->{"Windows_NT"} = {
    %$base,
    os => "Windows_NT",
    exec_suffix => '.exe',
    gdbargs	=> '-i=mi',
} ;

################################################################
$os->{"gnu-linux"} = {
    %$base,
    os => "gnu-linux",
    exec_suffix => '',
    drive	=> '/boo/e',
    gdbexec	=> 'gdb',
    gdbargs	=> '--annotate=3',
    jansson	=> '/home/nick/_/J/JSON/jansson-2.4',
} ;

################################################################
################################################################
$hostenv->{boo} = {
    %{$os->{"Windows_NT"}},
    drive	=> 'e:',
    gdbexec	=> 'd:/g/gdb-python-7.5-1/bin/gdb-python27.exe',
    jansson	=> 'e:/.p/_/J/JSON/jansson-2.4',
} ;

################################################################
$hostenv->{min} = {
    %{$os->{"Windows_NT"}},
    gdbexec	=> 'c:/g/gdb-python-7.5-1/bin/gdb-python27.exe',
    drive	=> 'j:',
    jansson	=> 'j:/_/J/JSON/jansson-2.4',
} ;

################################################################
$hostenv->{tyu} = {
    %{$os->{"Windows_NT"}},
    gdbexec	=> 'd:/g/gdb-python-7.5-1/bin/gdb-python27.exe',
    drive	=> 'e:',
    jansson	=> 'e:/_/J/JSON/jansson-2.4',
} ;

################################################################
$hostenv->{moga} = {
    %{$os->{"gnu-linux"}},
    drive	=> '/boo/e',
} ;

################################################################
$hostenv->{lube} = {
    %{$os->{"gnu-linux"}},
    drive	=> '/boo/e',
} ;

################################################################
$hostenv->{hop} = {
    %{$os->{"gnu-linux"}},
    drive	=> '/vom/e',
} ;

################################################################

1 ;
