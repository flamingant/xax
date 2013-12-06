################################################################
use npulgen ;

push @linkgroups,"AOBJSDEP" ;
push @linkgroups,"AOBJSNODEP" ;

macro "AEXE",'tc$(DOTEXE)' ;

push @targets,'$(AEXE)' ;
 
sub object_a { object @_,linkgroup => "AOBJSDEP"}

object '$(SQLITE)/$(ARCH)/sqlite3.o',linkgroup => "AOBJSNODEP" ;

object_a "main.c" ;
object_a "common.c" ;

1;
