include		locate.mk

ARCH :=		.bwg

VPATH =		..

CC :=		gcc

CCDBG :=	-ggdb 

WARN +=		-Wall \
		-Wno-switch \
		-Wno-missing-braces \
		-Wno-unused-function \
		-Wno-parentheses \
		-Werror-implicit-function-declaration \
		-Wno-format \
		-Wno-unused-but-set-variable

################################################################
CWARN =		$(WARN)

#CWARN +=	-Wimplicit-function-declaration

CWARN +=	\
		-Wno-pointer-sign

CINCLUDE +=	-I..
CINCLUDE +=	-I$(DEVROOT)
CINCLUDE +=	-I$(LIB_JANSSON)/src
CINCLUDE +=	-I$(SQLITE)

CFLAGS +=	$(CCDBG)

CDEFS_V +=
CDEFS_I +=	-DDEBUG=1 -D__$(@F:.o=_c)

.c.o:
		@perl -x.. ../ugen.pl gen $(<F)
		@echo $(CC) $(CFLAGS) $(CDEFS_V) -c $<
		@$(CC) $(CFLAGS) $(CWARN) $(CINCLUDE) $(CDEFS_I) $(CDEFS_V) -c $<
#		@echo $(CC) -x c++ $(CPPFLAGS) $(CDEFS_V) -c $<
#		@$(CC) -x c++ $(CPPFLAGS) $(CPPWARN) $(CINCLUDE) $(CDEFS_I) $(CDEFS_V) -c $<

################################################################
CPPFLAGS +=	$(CCDBG)
#CPPFLAGS +=	-fpermissive

CPPWARN =	$(WARN) \
		-Wno-write-strings \
		-Wno-sign-compare

.cpp.o:
		@perl -x.. ../ugen.pl gen $(<F)
		@echo $(CC) $(CPPFLAGS) $(CDEFS_V) -c $<
		@$(CC) $(CPPFLAGS) $(CPPWARN) $(CINCLUDE) $(CDEFS_I) $(CDEFS_V) -c $<

################################################################
#LDFLAGS +=	-mconsole
#LDFLAGS +=	-mno-cygwin -lmingw32

ALIBSDEP =	libnpu.a

ALIBS =		-L. -lnpu -L$(DEVROOT)/pu/$(ARCH) -lpu -L$(LIB_JANSSON)/src/.libs -llibjansson
ALIBS +=	-lcrypto
ALIBS +=	-lssl
ALIBS +=	-lssh2
ALIBS +=	-lstdc++

include lgen.mk

CPPSRC = 	main.o

$(CPPSRC) :	CC := $(CC) -x c++

$(CPPSRC) :	CFLAGS = $(CPPFLAGS)

$(CPPSRC) :	CWARN = $(CPPWARN)

AOBJSGEN =	$(AOBJSDEP)

AOBJSNODEP +=	gen.o

$(AEXE):	$(ALIBSDEP) $(AOBJSDEP) gen.o
		$(CC) -o $(AEXE) $(CCDBG) $(AOBJSDEP) $(AOBJSNODEP) $(ALIBS) $(LDFLAGS)

libnpu.a:	$(LIBNPU_OBJ)
		$(AR) cru $@ $(LIBNPU_OBJ)
		ranlib $@

lgen.mk:	lgen.pl ../lgen.pm ./lgen.pm
		perl -I.. lgen.pl make $@ ;

gen.c:		$(AOBJSGEN) lgen.mk
		perl -I.. lgen.pl init $@

ALLOBJSDEP =	$(AOBJSDEP)

clean:
		rm -f $(ALLOBJSDEP) $(AEXE) $(LIBNPU_OBJ) $(ALIBSDEP)

xclean:
		rm -f $(ALLOBJSDEP) $(AEXE) gen.o

squeaky:	clean
		rm -f ../.gen/*.c ../.gen/*.h ../.gen/*.g
		rm -f lgen.mk locate.mk .gdb.el .gdb/a gen.c

locate.mk:	locate.g.mk
		perl locate.g.mk >locate.mk

locate:		
		perl locate.g.mk >locate.mk
		perl .gdb.g.el >.gdb.el
		cd .gdb ; perl a.g >a
