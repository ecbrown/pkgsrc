# $NetBSD: Makefile,v 1.209 2026/08/06 14:06:58 adam Exp $

DISTNAME=	gnuplot-6.0.5
PKGREVISION=	1
CATEGORIES+=	graphics
MASTER_SITES=	${MASTER_SITE_SOURCEFORGE:=gnuplot/}

MAINTAINER=	pkgsrc-users@NetBSD.org
HOMEPAGE=	https://gnuplot.sourceforge.net/
COMMENT=	Portable interactive, function plotting utility
#LICENSE=	gnuplot + BSD

API_VERSION=	${PKGVERSION_NOREV:C/([0-9])\.([0-9])\.([0-9])+/\1.\2/}

CONFLICTS+=		gnuplot-nox11-[0-9]*

USE_LANGUAGES=		c
USE_TOOLS+=		pkg-config gmake
GNU_CONFIGURE=		yes
CONFIGURE_ARGS+=	--without-latex
CONFIGURE_ARGS+=	ac_cv_prog_EMACS=
TEST_TARGET=		check

PLIST_SUBST+=		API_VERSION=${API_VERSION}

.include "options.mk"

.if ${OPSYS} != "OpenVMS"
USE_TOOLS+=		makeinfo
INFO_FILES=		yes
.endif

.if ${OPSYS} == "OpenVMS"
USE_LANGUAGES+=		c++
CONFIGURE_ARGS+=	--with-readline=builtin
CONFIGURE_ARGS+=	--without-amos
CONFIGURE_ARGS+=	--disable-plugins
CONFIGURE_ENV+=		ac_cv_lib_z_deflate=no
CONFIGURE_ENV+=		ac_cv_func_mergesort=no
CONFIGURE_ENV+=		ac_cv_search_iconv_open=no
SUBST_CLASSES+=		openvms-paths
SUBST_STAGE.openvms-paths=	pre-configure
SUBST_FILES.openvms-paths=	src/syscfg.h
SUBST_VARS.openvms-paths=	PREFIX API_VERSION
MAKE_FLAGS+=		CC=${WRKDIR}/.wrapper/bin/cc
MAKE_FLAGS+=		CXX=${WRKDIR}/.wrapper/bin/cc
MAKE_FLAGS+=		CCLD=${WRKDIR}/.wrapper/bin/cc
MAKE_FLAGS+=		CXXLD=${WRKDIR}/.wrapper/bin/cc
MAKE_FLAGS+=		AR=/bin/ar.exe
MAKE_FLAGS+=		noinst_PROGRAMS=
MAKE_FLAGS+=		TERMLIBS=

OPENVMS_GNUPLOT_SOURCES=	alloc.c amos_airy.c axis.c breaders.c \
	boundary.c color.c command.c contour.c complexfun.c datablock.c \
	datafile.c dynarray.c encoding.c eval.c external.c filters.c fit.c \
	gadgets.c getcolor.c gplocale.c graph3d.c graphics.c help.c \
	hidden3d.c history.c internal.c interpol.c jitter.c libcerf.c \
	loadpath.c matrix.c misc.c mouse.c multiplot.c parse.c plot.c \
	plot2d.c plot3d.c pm3d.c readline.c save.c scanner.c set.c show.c \
	specfun.c standard.c stats.c stdfn.c tables.c tabulate.c term.c \
	time.c unset.c util.c util3d.c version.c vms.c voxelgrid.c vplot.c \
	watch.c xdg.c
OPENVMS_GNUPLOT_CPPFLAGS=	-DHAVE_CONFIG_H -DDECCRTL \
	-DGP_OPENVMS_POSIX -I. -I.. -I../term
OPENVMS_GNUPLOT_CFLAGS=	-O2 -Wc/PREFIX=ALL_ENTRIES
OPENVMS_GNUPLOT_DOC_CPPFLAGS=	-DHAVE_CONFIG_H -I. -I.. \
	-I../src -I../term -I../term/PostScript
OPENVMS_GNUPLOT_POSTSCRIPT=	8859-1.ps 8859-15.ps 8859-2.ps \
	8859-9.ps aglfn.txt cp1250.ps cp1251.ps cp1252.ps cp437.ps \
	cp850.ps cp852.ps koi8r.ps koi8u.ps prologue.ps utf-8.ps
OPENVMS_GNUPLOT_JS=	README canvasmath.js canvastext.js \
	gnuplot_common.js gnuplot_dashedlines.js gnuplot_mouse.css \
	gnuplot_mouse.js gnuplot_svg.js gnuplot_svg_2018.js grid.png \
	help.png nextzoom.png previouszoom.png return.png textzoom.png

# GNU make's Automake recursion emits shell commands larger than the GNV
# exec interface accepts, and its install recipes cannot reliably exec a
# shell from this process depth.  Drive compilation and installation here.
INSTALLATION_DIRS+=	bin man/man1 man/ja/man1
INSTALLATION_DIRS+=	share/gnuplot/${API_VERSION}/PostScript
INSTALLATION_DIRS+=	share/gnuplot/${API_VERSION}/js
INSTALLATION_DIRS+=	share/gnuplot/${API_VERSION}/lua

do-build:
	cd ${WRKSRC}/src && ${BUILD_MAKE_CMD} timestamp.h
.for source in ${OPENVMS_GNUPLOT_SOURCES}
	cd ${WRKSRC}/src && ${RM} -f ${source:R}.o && \
		/bin/dcl 'gcc ${OPENVMS_GNUPLOT_CPPFLAGS} ${OPENVMS_GNUPLOT_CFLAGS} -c -o ${source:R}.o ${source}' && \
		${TEST} -f ${source:R}.o
.endfor
	cd ${WRKSRC}/src && ${RM} -f libgnuplot.a gnuplot gnuplot.exe
.for source in ${OPENVMS_GNUPLOT_SOURCES}
.  if ${source} != "plot.c"
	cd ${WRKSRC}/src && /bin/ar.exe cr libgnuplot.a ${source:R}.o
.  endif
.endfor
	cd ${WRKSRC}/src && /bin/dcl \
		'link/executable=gnuplot.exe plot.o,libgnuplot.a/library' || \
		${TEST} -f ${WRKSRC}/src/gnuplot.exe
	cd ${WRKSRC}/src && /bin/ln.exe gnuplot.exe gnuplot
	cd ${WRKSRC}/docs && ${RM} -f doc2gih.o termdoc.o doc2gih doc2gih.exe gnuplot.gih
	cd ${WRKSRC}/docs && /bin/dcl \
		'gcc ${OPENVMS_GNUPLOT_DOC_CPPFLAGS} ${OPENVMS_GNUPLOT_CFLAGS} -c -o doc2gih.o doc2gih.c' && \
		${TEST} -f doc2gih.o
	cd ${WRKSRC}/docs && /bin/dcl \
		'gcc ${OPENVMS_GNUPLOT_DOC_CPPFLAGS} ${OPENVMS_GNUPLOT_CFLAGS} -c -o termdoc.o termdoc.c' && \
		${TEST} -f termdoc.o
	cd ${WRKSRC}/docs && /bin/dcl \
		'link/executable=doc2gih.exe doc2gih.o,termdoc.o' || \
		${TEST} -f ${WRKSRC}/docs/doc2gih.exe
	cd ${WRKSRC}/docs && /bin/ln.exe doc2gih.exe doc2gih
	cd ${WRKSRC}/docs && ./doc2gih ${WRKSRC}/docs/gnuplot.doc gnuplot.gih && \
		${TEST} -s gnuplot.gih

do-install:
	${INSTALL_PROGRAM} ${WRKSRC}/src/gnuplot \
		${DESTDIR}${PREFIX}/bin/gnuplot
	${INSTALL_MAN} ${WRKSRC}/man/man1/gnuplot.1 \
		${DESTDIR}${PREFIX}/man/man1/gnuplot.1
	${INSTALL_MAN} ${WRKSRC}/man/ja/man1/gnuplot.1 \
		${DESTDIR}${PREFIX}/man/ja/man1/gnuplot.1
.for file in ${OPENVMS_GNUPLOT_POSTSCRIPT}
	${INSTALL_DATA} ${WRKSRC}/term/PostScript/${file} \
		${DESTDIR}${PREFIX}/share/gnuplot/${API_VERSION}/PostScript/${file}
.endfor
	${INSTALL_DATA} ${WRKSRC}/share/colors_default.gp \
		${WRKSRC}/share/colors_mono.gp ${WRKSRC}/share/colors_podo.gp \
		${WRKSRC}/share/gnuplotrc ${WRKSRC}/docs/gnuplot.gih \
		${DESTDIR}${PREFIX}/share/gnuplot/${API_VERSION}
.for file in ${OPENVMS_GNUPLOT_JS}
	${INSTALL_DATA} ${WRKSRC}/term/js/${file} \
		${DESTDIR}${PREFIX}/share/gnuplot/${API_VERSION}/js/${file}
.endfor
	${INSTALL_DATA} ${WRKSRC}/term/lua/gnuplot-tikz.lua \
		${DESTDIR}${PREFIX}/share/gnuplot/${API_VERSION}/lua/gnuplot-tikz.lua
.else
.include "../../mk/readline.buildlink3.mk"
.  if ${READLINE_TYPE} == "editline"
CONFIGURE_ARGS+=	--with-readline=bsd
.  elif ${READLINE_TYPE} == "readline"
CONFIGURE_ARGS+=	--with-readline=gnu
.  endif
.endif

.include "../../converters/libiconv/buildlink3.mk"
.include "../../mk/curses.buildlink3.mk"
.include "../../mk/bsd.pkg.mk"
