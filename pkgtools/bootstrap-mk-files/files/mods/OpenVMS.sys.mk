# $NetBSD$
#
# System definitions for VSI OpenVMS under GNV.

unix?=		We run OpenVMS
OBJECT_FMT?=	ELF
UNAME?=		/bin/uname

.SUFFIXES: .out .a .o .s .S .c .cc .cpp .cxx .C .y .l .h .sh
.LIBS:		.a

AR?=		ar
ARFLAGS?=	cr
RANLIB?=	true

# GNV does not ship an assembler.  Keep AS distinct from CC so pkgsrc's
# wrapper de-duplication still creates the C compiler wrapper; the .s and .S
# suffix rules below intentionally continue to invoke the VSI C driver.
AS?=		false
AFLAGS?=
COMPILE.s?=	${CC} ${AFLAGS} -c
LINK.s?=	${CC} ${AFLAGS} ${LDFLAGS}
COMPILE.S?=	${CC} ${AFLAGS} ${CPPFLAGS} -c
LINK.S?=	${CC} ${AFLAGS} ${CPPFLAGS} ${LDFLAGS}

CC?=		gcc
DBG?=		-O2
CFLAGS?=	${DBG}
COMPILE.c?=	${CC} ${CFLAGS} ${CPPFLAGS} -c
LINK.c?=	${CC} ${CFLAGS} ${CPPFLAGS} ${LDFLAGS}

CXX?=		clang
CXXFLAGS?=	${CFLAGS}
COMPILE.cc?=	${CXX} ${CXXFLAGS} ${CPPFLAGS} -c
LINK.cc?=	${CXX} ${CXXFLAGS} ${CPPFLAGS} ${LDFLAGS}

CPP?=		${CC} -E
CPPFLAGS?=

INSTALL?=	install
LD?=		${CC}
LDFLAGS?=
LEX?=		lex
LFLAGS?=
LEX.l?=		${LEX} ${LFLAGS}
LORDER?=	echo
MAKE?=		make
NM?=		nm
SHELL?=		/bin/bash
SIZE?=		size
STRIP?=		true
TSORT?=		cat
YACC?=		yacc
YFLAGS?=
YACC.y?=	${YACC} ${YFLAGS}

.c:
	${LINK.c} -o ${.TARGET} ${.IMPSRC} ${LDLIBS}
.c.o:
	${COMPILE.c} ${.IMPSRC}
.c.a:
	${COMPILE.c} ${.IMPSRC}
	${AR} ${ARFLAGS} ${.TARGET} ${.PREFIX}.o
	rm -f ${.PREFIX}.o

.cc .cpp .cxx .C:
	${LINK.cc} -o ${.TARGET} ${.IMPSRC} ${LDLIBS}
.cc.o .cpp.o .cxx.o .C.o:
	${COMPILE.cc} ${.IMPSRC}
.cc.a .cpp.a .cxx.a .C.a:
	${COMPILE.cc} ${.IMPSRC}
	${AR} ${ARFLAGS} ${.TARGET} ${.PREFIX}.o
	rm -f ${.PREFIX}.o

.s:
	${LINK.s} -o ${.TARGET} ${.IMPSRC} ${LDLIBS}
.s.o:
	${COMPILE.s} ${.IMPSRC}
.S:
	${LINK.S} -o ${.TARGET} ${.IMPSRC} ${LDLIBS}
.S.o:
	${COMPILE.S} ${.IMPSRC}

.l:
	${LEX.l} ${.IMPSRC}
	${LINK.c} -o ${.TARGET} lex.yy.c ${LDLIBS} -ll
	rm -f lex.yy.c
.l.c:
	${LEX.l} ${.IMPSRC}
	mv lex.yy.c ${.TARGET}
.l.o:
	${LEX.l} ${.IMPSRC}
	${COMPILE.c} -o ${.TARGET} lex.yy.c
	rm -f lex.yy.c

.y:
	${YACC.y} ${.IMPSRC}
	${LINK.c} -o ${.TARGET} y.tab.c ${LDLIBS}
	rm -f y.tab.c
.y.c:
	${YACC.y} ${.IMPSRC}
	mv y.tab.c ${.TARGET}
.y.o:
	${YACC.y} ${.IMPSRC}
	${COMPILE.c} -o ${.TARGET} y.tab.c
	rm -f y.tab.c

.sh:
	rm -f ${.TARGET}
	cp ${.IMPSRC} ${.TARGET}
