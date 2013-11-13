################################################################
$os->{"Windows_NT"} = {
    os => "Windows_NT",
    exec_suffix => '.exe',
} ;

################################################################
$os->{"gnu-linux"} = {
    os => "gnu-linux",
    exec_suffix => '',
} ;

################################################################
################################################################
$hostenv->{boo} = {
    %{$os->{"Windows_NT"}},
    gdbexec	=> 'd:/g/gdb-python-7.5-1/bin/gdb-python27.exe',
    gdbargs	=> '-i=mi',
    drive	=> 'e:',
    master	=> 'e:/.p/.master',
    sqlite	=> 'e:/.p/.master/sqlite',
    jansson	=> 'e:/.p/_/J/JSON/jansson-2.4',
} ;

################################################################
$hostenv->{min} = {
    %{$os->{"Windows_NT"}},
    gdbargs	=> '-i=mi',
    gdbexec	=> 'c:/g/gdb-python-7.5-1/bin/gdb-python27.exe',
    drive	=> 'j:',
    master	=> 'j:/.master',
    sqlite	=> 'j:/.master/sqlite',
    jansson	=> 'j:/_/J/JSON/jansson-2.4',
} ;

################################################################
$hostenv->{tyu} = {
    %{$os->{"Windows_NT"}},
    gdbexec	=> 'd:/g/gdb-python-7.5-1/bin/gdb-python27.exe',
    gdbargs	=> '-i=mi',
    drive	=> 'e:',
    master	=> 'e:/.master',
    sqlite	=> 'e:/.master/sqlite',
    jansson	=> 'e:/_/J/JSON/jansson-2.4',
} ;

################################################################
$hostenv->{hop} = {
    %{$os->{"gnu-linux"}},
    drive	=> '/vom/e',
    gdbexec	=> 'gdb',
    gdbargs	=> '--annotate=3',
    master	=> '/vom/e/.master',
    sqlite	=> '/vom/e/.master/sqlite',
    jansson	=> '/home/nick/_/J/JSON/jansson-2.4',
} ;

################################################################

1 ;
