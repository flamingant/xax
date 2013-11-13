################################################################
push @linkgroups,"AOBJSDEP" ;
push @linkgroups,"AOBJSNODEP" ;

macro "AEXE",'c$(DOTEXE)' ;

push @targets,'$(AEXE)' ;
 
sub object_a { object @_,linkgroup => "AOBJSDEP"}

object_a "main.c" ;
object_a "common.c" ;

object_a "cpp.cpp" ;

1;
