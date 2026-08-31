# $NetBSD: Makefile,v 1.124 2026/01/20 10:07:04 wiz Exp $

DISTNAME=	ncurses-6.6
CATEGORIES=	devel
MASTER_SITES=	${MASTER_SITE_GNU:=ncurses/}

MAINTAINER=	pkgsrc-users@NetBSD.org
HOMEPAGE=	https://invisible-island.net/ncurses/ncurses.html
LICENSE=	mit
COMMENT=	CRT screen handling and optimization package

CONFLICTS+=	ncursesw-[0-9]*
SUPERSEDES+=	ncursesw-[0-9]*

USE_LANGUAGES=		c c++
USE_LIBTOOL=		yes
USE_TOOLS+=		gmake pkg-config tbl
FORCE_C_STD=		gnu99

GNU_CONFIGURE=		yes
CONFIGURE_ARGS+=	--with-libtool
CONFIGURE_ARGS+=	--enable-bsdpad
CONFIGURE_ARGS+=	--enable-widec
CONFIGURE_ARGS+=	--disable-overwrite
CONFIGURE_ARGS+=	--disable-lib-suffixes
CONFIGURE_ARGS+=	--with-cxx-binding
CONFIGURE_ARGS+=	--enable-pc-files
CONFIGURE_ARGS+=	--with-pkg-config-libdir=${PREFIX}/lib/pkgconfig
CONFIGURE_ARGS+=	--without-ada
CONFIGURE_ARGS+=	--without-gpm
CONFIGURE_ARGS+=	--with-manpage-format=normal
CONFIGURE_ARGS+=	--without-manpage-renames
CONFIGURE_ARGS+=	--with-default-terminfo-dir=${PREFIX}/${TERMINFODIR}
CONFIGURE_ARGS+=	--with-terminfo-dirs=${PREFIX}/${TERMINFODIR}

# These libraries are renamed as there were apparently issues back in 2016 when
# they matched the same names as builtin NetBSD libraries.  It would be good to
# verify that this is still a problem
#
CONFIGURE_ARGS+=	--with-form-libname=gnuform
CONFIGURE_ARGS+=	--with-menu-libname=gnumenu
CONFIGURE_ARGS+=	--with-panel-libname=gnupanel

CONFIGURE_ARGS.Darwin+=	--disable-mixed-case
CONFIGURE_ARGS.SunOS+=	--disable-const

CONFIGURE_ARGS.UnixWare+=	--disable-libtool-version

# QNX libc is missing some functions.
CONFIGURE_ARGS.QNX+=	--disable-widec

TERMINFODIR=		share/terminfo

SUBST_CLASSES+=		rpath
SUBST_FILES.rpath+=	misc/ncurses-config
SUBST_STAGE.rpath=	post-configure
SUBST_SED.rpath+=	-e "s|-L\\(.libdir\\)|${COMPILER_RPATH_FLAG}\\1 -L\\1|"
SUBST_MESSAGE.rpath=	Fixing rpath in ncurses-config.

# GNV Bash can lose its current directory after config.status returns.  Run
# ncurses' final summary target from the configure script's saved directory.
SUBST_CLASSES.OpenVMS+=	preinstall
SUBST_STAGE.preinstall=	pre-configure
SUBST_FILES.preinstall=	configure
SUBST_SED.preinstall=	-e 's|$${MAKE:-make} preinstall|$${MAKE:-make} -C "${WRKSRC}" -f Makefile preinstall|'

# GNV make does not preserve command-line variables in ncurses' recursive
# makes, and its child shells cannot reliably rediscover the libtool wrapper.
# Avoid Bash's simulated `(cd ... && make)` subshells as well: descriptors
# inherited through those subshells make later exec calls fail intermittently.
SUBST_CLASSES.OpenVMS+=	recursive-libtool
SUBST_STAGE.recursive-libtool=	post-configure
SUBST_FILES.recursive-libtool=	Makefile include/Makefile ncurses/Makefile
SUBST_FILES.recursive-libtool+=	progs/Makefile panel/Makefile menu/Makefile
SUBST_FILES.recursive-libtool+=	form/Makefile misc/Makefile
SUBST_SED.recursive-libtool=	-e 's|^TOP_MFLAGS[[:space:]]*= |TOP_MFLAGS = LIBTOOL="${WRAPPER_DIR}/bin/libtool" |'
SUBST_SED.recursive-libtool+=	-e 's|( cd \([^ ]*\) && $${MAKE} $${TOP_MFLAGS} $$@ )|$${MAKE} -C \1 $${TOP_MFLAGS} $$@|'
SUBST_SED.recursive-libtool+=	-e 's|( cd \([^ ]*\) && $$(MAKE)|$$(MAKE) -C \1|'
SUBST_SED.recursive-libtool+=	-e '/$$(MAKE) -C /s| )$$||'
SUBST_SED.recursive-libtool+=	-e 's|@ (cd $${INCLUDEDIR} && rm -f \([^)]*\)) ;|@ rm -f $${INCLUDEDIR}/\1 ;|'
SUBST_SED.recursive-libtool+=	-e 's|( cd $$(BINDIR) && $$(LN_S) $$(actual_tic) $$(actual_infotocap) )|$$(LN_S) $$(actual_tic) $$(BINDIR)/$$(actual_infotocap)|'
SUBST_SED.recursive-libtool+=	-e 's|( cd $$(BINDIR) && $$(LN_S) $$(actual_tic) $$(actual_captoinfo) )|$$(LN_S) $$(actual_tic) $$(BINDIR)/$$(actual_captoinfo)|'
SUBST_SED.recursive-libtool+=	-e 's|( cd $$(BINDIR) && $$(LN_S) $$(actual_tset) $$(actual_reset) )|$$(LN_S) $$(actual_tset) $$(BINDIR)/$$(actual_reset)|'
SUBST_SED.recursive-libtool+=	-e 's|@ (cd $${INCLUDEDIR} && rm -f ncurses.h && $${LN_S} curses.h ncurses.h)|@ rm -f $${INCLUDEDIR}/ncurses.h ; $${LN_S} curses.h $${INCLUDEDIR}/ncurses.h|'
SUBST_SED.recursive-libtool+=	-e 's|(cd $${INCLUDEDIR} && rm -f \([^)]*\))|rm -f $${INCLUDEDIR}/\1|'
SUBST_SED.recursive-libtool+=	-e 's|@( cd $$(srcdir)/tabset &&|@cd $$(srcdir)/tabset ;|'
SUBST_SED.recursive-libtool+=	-e "s|fi; done' )|fi; done'|"

SUBST_CLASSES+=		setenv
SUBST_FILES.setenv=	misc/run_tic.in
SUBST_STAGE.setenv=	pre-configure
SUBST_VARS.setenv=	SETENV

.include "../../mk/bsd.prefs.mk"

PLIST_VARS+=		cxx static
.if ${OPSYS} == "OpenVMS"
# The VSI x86-64 kit currently supplies C but no working C++ frontend.  The
# C++ binding and test programs are not needed by C consumers such as Lynx.
USE_LANGUAGES:=		c
CONFIGURE_ARGS+=	--without-cxx --without-cxx-binding --without-tests
CONFIGURE_ARGS+=	--disable-ext-colors
CONFIGURE_ARGS+=	--enable-symlinks
CONFIGURE_ENV+=		cf_cv_have_tcgetattr=yes
# Starting a sub-make from another GNV make leaves Bash with descriptors that
# make a later exec fail with "illegal seek".  Have pkgsrc invoke each ncurses
# directory directly, preserving the upstream dependency order.
BUILD_DIRS=		man include ncurses progs panel menu form misc
INSTALL_DIRS=		man include ncurses progs panel menu form misc
MAKE_FLAGS+=		LIBTOOL=${WRAPPER_DIR}/bin/libtool
# The configured program-name transform is a no-op, but the generated
# makefiles recompute every name with command substitutions and sed pipelines.
# GNV Bash leaks resources while emulating those subshells and can then fail to
# exec the compiler.  Pass the already-resolved names through recursive makes.
MAKE_FLAGS+=		actual_captoinfo=captoinfo actual_clear=clear
MAKE_FLAGS+=		actual_infocmp=infocmp actual_infotocap=infotocap
MAKE_FLAGS+=		actual_init=init actual_reset=reset actual_tabs=tabs
MAKE_FLAGS+=		actual_tic=tic actual_toe=toe actual_tput=tput
MAKE_FLAGS+=		actual_tset=tset
MAKE_FLAGS+=		define_captoinfo=captoinfo define_clear=clear
MAKE_FLAGS+=		define_infotocap=infotocap define_init=init
MAKE_FLAGS+=		define_reset=reset
# GNV awk does not reliably capture libtool-expand's child output, so list the
# static archive payloads explicitly instead of relying on PLIST expansion.
PLIST.static=		yes
.else
PLIST.cxx=		yes
.endif

.if ${USE_CROSS_COMPILE:U:tl} == yes
TOOL_DEPENDS+=		ncurses-[0-9]*:../../devel/ncurses
CONFIGURE_ENV+=		BUILD_CC=${NATIVE_CC:Q}
CONFIGURE_ENV+=		ac_cv_path_TIC_PATH=${TOOLBASE:Q}/bin/tic
ALL_ENV+=		TIC_PATH=${TOOLBASE:Q}/bin/tic
.endif

# If there's no native tbl(1) then the tools infrastructure would pull in
# groff, causing various circular dependencies.
.if defined(TOOLS_TBL)
CONFIGURE_ARGS+=	--with-manpage-tbl
.else
USE_TOOLS:=		${USE_TOOLS:Ntbl}
CONFIGURE_ARGS+=	--without-manpage-tbl
.endif

.include "../../mk/compiler.mk"

# Fix build under FreeBSD 10.2
.if !empty(PKGSRC_COMPILER:Mclang)
CONFIGURE_ENV+=		ac_cv_c_compiler_gnu=no
CONFIGURE_ENV+=		ac_cv_cxx_compiler_gnu=no
.endif

.if ${CC_VERSION:Mgcc-5.*}
# Inhibit generation of line markers in the output from the preprocessor
# https://gcc.gnu.org/bugzilla/show_bug.cgi?id=60723
CPPFLAGS+=	-P
.endif

# This prevents tic from writing into ~/.terminfo
INSTALLATION_DIRS+=	${TERMINFODIR}

# TERMINFODIR entries are generated automatically.
PLIST_SRC=		${PKGDIR}/PLIST ${WRKDIR}/PLIST.terminfo
PRINT_PLIST_AWK+=	/${TERMINFODIR:S|/|\\/|g}/ { next; }

CHECK_WRKREF_SKIP+=	man/man?/*

post-configure:
	cd ${WRKSRC}/man;					\
	for f in *.1m; do					\
		mv -f $${f} `${BASENAME} $${f} .1m`.1; 	\
	done;							\
	for f in *.3x; do					\
		mv -f $${f} `${BASENAME} $${f} .3x`.3; 	\
	done

post-install:
.if ${OPSYS} == "Darwin"
	# Clashes with "a" versions on case-insensitive file systems
	${RM} -f ${DESTDIR}${PREFIX}/${TERMINFODIR}/32/2621A
	${RM} -f ${DESTDIR}${PREFIX}/${TERMINFODIR}/68/hp2621A
	${RM} -f ${DESTDIR}${PREFIX}/${TERMINFODIR}/68/hp70092A
.endif
.if ${OPSYS} == "OpenVMS"
	${FIND} ${DESTDIR}${PREFIX}/${TERMINFODIR} -type f -print -o \
		-type l -print | ${SORT} | \
		${SED} -e "s,^${DESTDIR}${PREFIX}/,," >${WRKDIR}/PLIST.terminfo
.else
	${FIND} ${DESTDIR}${PREFIX}/${TERMINFODIR} -type f -print | ${SORT} | \
		${SED} -e "s,^${DESTDIR}${PREFIX}/,," >${WRKDIR}/PLIST.terminfo
.endif
	${RM} -f ${DESTDIR}${PREFIX}/lib/terminfo

.include "../../mk/bsd.pkg.mk"
