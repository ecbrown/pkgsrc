# $NetBSD: Makefile,v 1.42 2026/08/24 11:14:20 adam Exp $

DISTNAME=	pkgconf-3.0.6
CATEGORIES=	devel
MASTER_SITES=	https://distfiles.dereferenced.org/pkgconf/

MAINTAINER=	nico@orgrim.net
HOMEPAGE=	https://github.com/pkgconf/pkgconf
COMMENT=	API-driven pkg-config replacement
LICENSE=	modified-bsd

TEST_DEPENDS+=	kyua-[0-9]*:../../devel/kyua

CONFLICTS+=	pkg-config-[0-9]*

USE_LIBTOOL=		yes
GNU_CONFIGURE=		yes
CONFIGURE_ARGS+=	--with-system-libdir=/usr/lib
CONFIGURE_ARGS+=	--with-system-includedir=/usr/include

# VSI C does not implement GCC visibility attributes.  The OpenVMS libtool
# backend builds libpkgconf statically, for which the public API needs none.
CPPFLAGS.OpenVMS+=	-DPKGCONFIG_IS_STATIC

# GNV tar cannot preserve Automake's distributed timestamp ordering.  Keep
# the build on the shipped generated files and out of dependency tracking.
CONFIGURE_ARGS.OpenVMS+=	--disable-dependency-tracking
MAKE_FLAGS.OpenVMS+=	ACLOCAL=true AUTOCONF=true
MAKE_FLAGS.OpenVMS+=	AUTOHEADER=true AUTOMAKE=true
# The non-installed test runner requires mkdtemp(), which is not provided by
# the VSI C RTL.  Keep it out of the normal production build.
MAKE_FLAGS.OpenVMS+=	noinst_PROGRAMS=

MAKE_FLAGS+=	MANDIR=${PREFIX}/${PKGMANDIR}/man1

PKGCONFIG_PATHS=	${PREFIX}/lib/pkgconfig
PKGCONFIG_PATHS+=	${PREFIX}/share/pkgconfig
PKGCONFIG_PATHS+=	/usr/lib/pkgconfig

TEST_TARGET=	check

.include "../../mk/bsd.prefs.mk"

.if ${LOCALBASE} != ${X11BASE}
PKGCONFIG_PATHS+=	${X11BASE}/lib/pkgconfig
.endif

.if ${OPSYS} == "Linux"
PKGCONFIG_PATHS+=	/usr/share/pkgconfig
.endif

CONFIGURE_ARGS+=	--with-pkg-config-dir=${PKGCONFIG_PATHS:ts::Q}

# configure script incorrectly sets only _FILE_OFFSET_BITS=64
CPPFLAGS.SunOS+=	-D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE

# readlinkat() arrived in Mac OS X 10.10 Yosemite
.if ${OPSYS} == "Darwin" && ${OPSYS_VERSION} < 101000
LDFLAGS+=		-lMacportsLegacySupport
.  include "../../devel/macports-legacy-support/buildlink3.mk"
.endif

post-install:
	${LN} -s pkgconf ${DESTDIR}${PREFIX}/bin/pkg-config
	${LN} -s pkgconf.1 ${DESTDIR}${PREFIX}/${PKGMANDIR}/man1/pkg-config.1

.include "../../mk/bsd.pkg.mk"
# PKG_CONFIG* variables in test environment break tests
TEST_ENV=	HOME=${FAKEHOMEDIR}
