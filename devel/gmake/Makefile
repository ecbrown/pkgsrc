# $NetBSD: Makefile,v 1.127 2025/04/09 07:55:55 nia Exp $

DISTNAME=	make-4.4.1
PKGNAME=	g${DISTNAME}
CATEGORIES=	devel
MASTER_SITES=	${MASTER_SITE_GNU:=make/}

MAINTAINER=	pkgsrc-users@NetBSD.org
HOMEPAGE=	https://www.gnu.org/software/make/make.html
COMMENT=	GNU version of 'make' utility
LICENSE=	gnu-gpl-v3

USE_FEATURES=		snprintf

GNU_CONFIGURE=		yes
TEXINFO_REQD+=		4.0
INFO_FILES=		yes

# GNV tar cannot preserve the distributed timestamp ordering on read-only
# archive members.  Avoid maintainer-tool regeneration and Automake's
# dependency-fragment bootstrap on OpenVMS.
CONFIGURE_ARGS.OpenVMS+=	--disable-dependency-tracking --disable-job-server
# The native VMS backend waits through LIB$SPAWN and does not provide the
# obsolete siggetmask() companion for the C RTL's sigsetmask().
CONFIGURE_ENV.OpenVMS+=	ac_cv_func_sigsetmask=no
MAKE_FLAGS.OpenVMS+=	ACLOCAL=true AUTOCONF=true
MAKE_FLAGS.OpenVMS+=	AUTOHEADER=true AUTOMAKE=true

# Gnulib's legacy OpenVMS implementation expects the optional LAV0 load-average
# driver and leaves LOAD_AVE_TYPE undefined on current VSI systems.  GNU make
# already handles an unavailable load average, so compile its portable stub.
# Output synchronization belongs to posixos.c, which the VMS backend bypasses.
CPPFLAGS.OpenVMS+=	-DNO_GET_LOAD_AVG -DNO_OUTPUT_SYNC

# The Autoconf build only distinguishes Windows from "other" and therefore
# selects posixos.c on OpenVMS.  Use the VMS source set that upstream's native
# mk/VMS.mk build selects; Automake already emits compilation rules for it.
SUBST_CLASSES.OpenVMS+=	vmssrc
SUBST_STAGE.vmssrc=	pre-configure
SUBST_FILES.vmssrc=	Makefile.in
SUBST_SED.vmssrc=	-e 's|@WINDOWSENV_FALSE@am__append_4 = src/posixos.c|@WINDOWSENV_FALSE@am__append_4 = $$(vms_SRCS)|'
SUBST_SED.vmssrc+=	-e 's|@WINDOWSENV_FALSE@am__objects_4 = src/posixos.$$(OBJEXT)|@WINDOWSENV_FALSE@am__objects_4 = src/vms_exit.$$(OBJEXT) src/vms_export_symbol.$$(OBJEXT) src/vms_progname.$$(OBJEXT) src/vmsfunctions.$$(OBJEXT) src/vmsify.$$(OBJEXT)|'
# Do not let Automake replace the distributed Info file when makeinfo is not
# available yet; GNU make is itself part of the toolchain needed to build it.
SUBST_SED.vmssrc+=	-e 's|^SUBDIRS = lib po doc$$|SUBDIRS = lib po|'

# GNV patch cannot reliably traverse the form-feed-delimited sections in this
# source files.  Apply the three small, exact signal changes with SUBST.
SUBST_CLASSES.OpenVMS+=	vmssig
SUBST_STAGE.vmssig=	pre-configure
SUBST_FILES.vmssig=	src/main.c src/vmsjobs.c
SUBST_SED.vmssig=	-e 's,elif defined(HAVE_SIGSETMASK),elif defined(HAVE_SIGSETMASK) || defined(VMS),'
SUBST_SED.vmssig+=	-e 's,ifdef  HAVE_SIGSETMASK,if defined(HAVE_SIGSETMASK) || defined(VMS),'
SUBST_SED.vmssig+=	-e 's,static int vms_jobsefnmask = 0;,extern int fatal_signal_mask; static int vms_jobsefnmask = 0;,'

TEST_TARGET=		check
USE_TOOLS+=		perl:test

CONFIGURE_ARGS+=	--program-prefix=g

# XXX: Do not install gmake setgid and do not override the owner group
# XXX: based on possible /dev/kmem permissions.
CONFIGURE_ARGS+=	ac_cv_func_getloadavg_setgid=no

.include "../../mk/bsd.prefs.mk"

#
# Keep in sync with devel/bmake, the goal is to ensure we use the same shell
# that was selected by bootstrap.
#
.if ${TOOLS_PLATFORM.sh:M?*/bin/mksh}
DEPENDS+=	mksh-[0-9]*:../../shells/mksh
CPPFLAGS+=	-DPKGSRC_DEFAULT_SHELL="\"${PREFIX}/bin/mksh\""
.elif ${TOOLS_PLATFORM.sh:M?*/bin/pdksh}
DEPENDS+=	pdksh-[0-9]*:../../shells/pdksh
CPPFLAGS+=	-DPKGSRC_DEFAULT_SHELL="\"${PREFIX}/bin/pdksh\""
.elif (${OPSYS} == "SunOS" || ${OPSYS} == "UnixWare" || \
	${OPSYS} == "SCO_SV") && exists(/usr/bin/bash)
CPPFLAGS+=	-DPKGSRC_DEFAULT_SHELL="\"/usr/bin/bash\""
.endif

# kernel bug, fixed in NetBSD 9.99.82
.if ${OPSYS} == "NetBSD" && ${OPSYS_VERSION} < 099982
CONFIGURE_ARGS.NetBSD+=	--disable-posix-spawn
.endif

.if ${OPSYS} == "UnixWare"
# avoid colliding with libc functions
SUBST_CLASSES+=		strlist
SUBST_STAGE.strlist=	pre-configure
SUBST_FILES.strlist=	src/main.c
SUBST_SED.strlist=	-e 's/strlist/strlist_make/g'
.endif

.include "options.mk"

AUTO_MKDIRS=		yes

post-install:
	${LN} -sf ${PREFIX}/bin/gmake ${DESTDIR}${PREFIX}/${PKGGNUDIR}bin/make
	${LN} -sf ${PREFIX}/${PKGMANDIR}/man1/gmake.1 ${DESTDIR}${PREFIX}/${PKGGNUDIR}${PKGMANDIR}/man1/make.1
.if ${OPSYS} == "OpenVMS"
	${INSTALL_DATA_DIR} ${DESTDIR}${PREFIX}/${PKGINFODIR}
	${INSTALL_DATA} ${WRKSRC}/doc/make.info ${DESTDIR}${PREFIX}/${PKGINFODIR}/make.info
	${INSTALL_MAN_DIR} ${DESTDIR}${PREFIX}/${PKGMANDIR}/man1
	${INSTALL_MAN} ${WRKSRC}/doc/make.1 ${DESTDIR}${PREFIX}/${PKGMANDIR}/man1/gmake.1
.endif

.include "../../mk/bsd.pkg.mk"
