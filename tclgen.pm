################################################################
push @linkgroups,"AOBJSDEP" ;
push @linkgroups,"AOBJSNODEP" ;

macro "AEXE",'tc$(DOTEXE)' ;

push @targets,'$(AEXE)' ;
 
sub object_a { object @_,linkgroup => "AOBJSDEP"}

object '$(SQLITE)/$(ARCH)/sqlite3.o',linkgroup => "AOBJSNODEP" ;

object_a "main.c" ;
object_a "common.c" ;
object_a "log.c" ;
object_a "atinit.c" ;

object_a "ma.c" ;
object_a "ma_irc.c" ;

object_a "http.c" ;
object_a "ht_tc.c" ;

object_a "dht.c" ;
object_a "dht_session.c" ;
object_a "dht_label.c" ;
object_a "dht_move.c" ;
#object_a "dht_alpha.c" ;

object_a "ht_what.c" ;
object_a "what_group.c" ;
object_a "what_search.c" ;

object_a "uf.c" ;
object_a "ufi.c" ;

object_a "tc_sql.c" ;
#object_a "ssl.c" ;

object_a "ssh_scp.c" ;
#object_a "ssh_sh.c" ;
object_a "ssh.c" ;

object_a "feral.c" ;
object_a "inserver.c" ;
object_a "winserv.c" ;

object_a "uncollect.c" ;

object_a "cpp.cpp" ;
1;
