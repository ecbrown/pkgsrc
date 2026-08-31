# $NetBSD: Makefile,v 1.84 2026/04/07 12:23:36 wiz Exp $

DISTNAME=	m4-1.4.21
CATEGORIES=	devel
MASTER_SITES=	${MASTER_SITE_GNU:=m4/}

MAINTAINER=	pkgsrc-users@NetBSD.org
HOMEPAGE=	https://www.gnu.org/software/m4/
COMMENT=	GNU version of UNIX m4 macro language processor
LICENSE=	gnu-gpl-v3

GNU_CONFIGURE=		yes
CONFIGURE_ARGS+=	--disable-nls
CONFIGURE_ARGS+=	--program-prefix=g
CONFIGURE_ARGS.Cygwin+=	ac_cv_func___fpending=yes
CONFIGURE_ARGS.OpenVMS+=	--disable-dependency-tracking

# GNV tar cannot restore timestamps on read-only archive members.  Its
# --touch workaround makes Automake inputs and generated release files appear
# in extraction order, so prevent unnecessary maintainer-tool regeneration.
MAKE_FLAGS.OpenVMS+=	ACLOCAL=true AUTOCONF=true
MAKE_FLAGS.OpenVMS+=	AUTOHEADER=true AUTOMAKE=true
MAKE_FLAGS.OpenVMS+=	MAKEINFO=true HELP2MAN=true

SUBST_CLASSES.OpenVMS+=	floatplus
SUBST_STAGE.floatplus=	post-patch
SUBST_FILES.floatplus=	lib/isnan.c lib/signbitd.c
SUBST_FILES.floatplus+=	lib/signbitf.c lib/signbitl.c
SUBST_FILES.floatplus+=	lib/vasnprintf.c
SUBST_SED.floatplus=	-e 's/float+\.h/float-plus.h/g'

# decc$set_child_standard_streams cannot redirect a pipe created with
# O_CLOEXEC.  The OpenVMS spawni patch marks only the unmapped peer endpoint.
SUBST_CLASSES.OpenVMS+=	spawnpipe
SUBST_STAGE.spawnpipe=	post-patch
SUBST_FILES.spawnpipe=	lib/spawn-pipe.c
SUBST_SED.spawnpipe=	-e 's/O_BINARY | O_CLOEXEC/O_BINARY/g'

# The gnulib test programs are exercised by the check target and are not
# installed.  Do not make them a prerequisite of the normal OpenVMS build.
SUBST_CLASSES.OpenVMS+=	notests
SUBST_STAGE.notests=	post-patch
SUBST_FILES.notests=	Makefile.in
SUBST_SED.notests=	-e 's/ po tests/ po/'
INFO_FILES=		yes
TEST_TARGET=		check

.include "../../mk/bsd.prefs.mk"

CFLAGS.AIX+=	-D_LINUX_SOURCE_COMPAT

.if ${MACHINE_ARCH} == "i386"
CFLAGS.QNX+=		-march=i486  # Defaults to i386 otherwise, which lacks atomics
CFLAGS.UnixWare+=	-march=i486  # Defaults to i386 otherwise, which lacks atomics
.endif

AUTO_MKDIRS=		yes

.if ${OPSYS} == "OpenVMS"
post-extract:
	${MV} ${WRKSRC}/lib/float+.h ${WRKSRC}/lib/float-plus.h
.endif

.if ${OPSYS} == "UnixWare"
# cannot handle stat functions defined as macros by uw7 libc
SUBST_CLASSES+=		offset
SUBST_STAGE.offset=	pre-configure
SUBST_FILES.offset=	configure
SUBST_SED.offset=	-e 's/FILE_OFFSET_BITS 64/FILE_OFFSET_BITS 32/g'
.endif

post-install:
	${INSTALL_DATA} ${WRKSRC}/examples/*.m4 ${DESTDIR}${PREFIX}/share/examples/m4
	${LN} -sf ${PREFIX}/bin/gm4 ${DESTDIR}${PREFIX}/${PKGGNUDIR}bin/m4
	${LN} -sf ${PREFIX}/${PKGMANDIR}/man1/gm4.1 ${DESTDIR}${PREFIX}/${PKGGNUDIR}${PKGMANDIR}/man1/m4.1

.include "../../mk/bsd.pkg.mk"
