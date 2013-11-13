include		locate.mk

ARCH :=		.bln

VPATH =		..

CC :=		gcc

CCDBG :=	-ggdb 
#CWARN +=	-Wimplicit-function-declaration
CWARN +=	-Wall \
		-Wno-switch \
		-Wno-missing-braces \
		-Wno-unused-function \
		-Wno-parentheses \
		-Werror-implicit-function-declaration \
		-Wno-format \
		-Wno-pointer-sign \
		-Wno-unused-but-set-variable

CINCLUDE +=	-I..
CINCLUDE +=	-I$(DEVROOT)
CINCLUDE +=	-I$(LIB_JANSSON)/src
CINCLUDE +=	-I$(SQLITE)

CFLAGS +=	$(CCDBG)

CDEFS_V +=
CDEFS_I +=	-DDEBUG=1 -D__$(@F:.o=_c)

.c.o:
		@perl -I../.gen/.tools ../ugen.pl gen $<
		@echo $(CC) $(CFLAGS) $(CDEFS_V) -c $<
		@$(CC) $(CFLAGS) $(CINCLUDE) $(CDEFS_I) $(CDEFS_V) $(CWARN) -c $<

#LDFLAGS +=	-mconsole
#LDFLAGS +=	-mno-cygwin -lmingw32

ALIBSDEP =	libnpu.a

ALIBS =		-L. -lnpu -L$(DEVROOT)/pu/$(ARCH) -lpu /usr/local/lib/libjansson.a -lrt
ALIBS +=	-lcrypto
ALIBS +=	-lssl
ALIBS +=	-lssh2

include lgen.mk

AOBJSGEN =	$(AOBJSDEP)

AOBJSNODEP +=	$(SQLITE)/$(ARCH)/sqlite3.o gen.o

$(AEXE):	$(ALIBSDEP) $(AOBJSDEP) gen.o
		$(CC) -o $(AEXE) $(CCDBG) $(AOBJSDEP) $(AOBJSNODEP) $(ALIBS) $(LDFLAGS)

libnpu.a:	$(LIBNPU_OBJ)
		$(AR) cru $@ $(LIBNPU_OBJ)
		ranlib $@

lgen.mk:	lgen.pl ../lgen.pm ./lgen.pm
		perl lgen.pl make $@ ;

gen.c:		$(AOBJSGEN) lgen.mk
		perl lgen.pl init $@

clean:
		rm -f $(AOBJSDEP) $(AEXE) $(LIBNPU_OBJ) $(ALIBSDEP) gen.o

squeaky:	clean
		rm -f ../.gen/*.c ../.gen/*.h ../.gen/*.g
		rm -f lgen.mk locate.mk .gdb.el .gdb/a gen.c

locate.mk:	locate.g.mk
		perl locate.g.mk >locate.mk

locate:		
		perl locate.g.mk >locate.mk
		perl .gdb.g.el >.gdb.el
		cd .gdb ; perl a.g >a
