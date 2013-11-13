push @linkgroups,"LIBNPU_OBJ" ;
sub object_l { object @_,linkgroup => "LIBNPU_OBJ"}

object_l "gunzip.c" ;

object_l "arg.c" ;
object_l "errors.c" ;
object_l "dp.c" ;
object_l "stimer.c" ;
#object_l "bencode.c" ;
#object_l "sha1.c" ;

object_l "dts.c" ;

object_l "sql.c" ;
object_l "vcf.c" ;

object_l "jsf.c" ;
object_l "hashtab.c" ;
object_l "tod.c" ;

object_l "gmalloc.c" ;
object_l "glt.c" ;

use tclgen ;
#use clgen ;

################################################################
1 ;
