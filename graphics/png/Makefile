# $NetBSD: Makefile,v 1.224 2026/05/17 13:50:09 tnn Exp $

DISTNAME=	libpng-1.6.58
PKGREVISION=	1
PKGNAME=	${DISTNAME:S/lib//}
CATEGORIES=	graphics
MASTER_SITES+=	${MASTER_SITE_SOURCEFORGE:=libpng/}
EXTRACT_SUFX=	.tar.xz

MAINTAINER=	pkgsrc-users@NetBSD.org
HOMEPAGE=	https://github.com/pnggroup/libpng
COMMENT=	Library for manipulating PNG images
LICENSE=	zlib

USE_LIBTOOL=		yes
GNU_CONFIGURE=		yes
# do not create libpng.la
CONFIGURE_ARGS+=	--disable-unversioned-links
# but do install png.h outside include/libpng$VERSION for now
# until pkgsrc is fixed to use libpng-config or the pc file everywhere
INSTALL_TARGET=		install install-header-links

PKGCONFIG_OVERRIDE=	scripts/libpng.pc.in libpng.pc.in
TEST_TARGET=		check

CPPFLAGS+=	-DPNG_USER_CHUNK_CACHE_MAX=128 -DPNG_USER_CHUNK_MALLOC_MAX=8000000

CPPFLAGS.UnixWare+=	-D_XOPEN_SOURCE=500 -D_POSIX_C_SOURCE=199506

.include "../../mk/bsd.prefs.mk"
.include "options.mk"

PLIST_VARS+=		config libtool static tools

.if ${OPSYS} == "OpenVMS"
# OpenVMS uses static object libraries.  The upstream distribution ships a
# pre-generated public configuration header specifically for non-Autoconf
# builds, so no host tools are needed here.
.undef USE_LIBTOOL
.undef GNU_CONFIGURE
NO_CONFIGURE=		yes
PLIST.config=		yes
PLIST.static=		yes

OPENVMS_PNG_SOURCES=	png.c pngerror.c pngget.c pngmem.c pngpread.c \
	pngread.c pngrio.c pngrtran.c pngrutil.c pngset.c pngtrans.c \
	pngwio.c pngwrite.c pngwtran.c pngwutil.c
OPENVMS_PNG_CPPFLAGS=	-DPNG_ARM_NEON_OPT=0 -DPNG_MIPS_MSA_OPT=0 \
	-DPNG_POWERPC_VSX_OPT=0 -DPNG_INTEL_SSE_OPT=0 \
	-DPNG_RISCV_RVV_OPT=0

INSTALLATION_DIRS=	bin include include/libpng16 lib lib/pkgconfig \
	${PKGMANDIR}/man3 ${PKGMANDIR}/man5

post-extract:
	${CP} ${WRKSRC}/scripts/pnglibconf.h.prebuilt \
		${WRKSRC}/pnglibconf.h

do-build:
.for source in ${OPENVMS_PNG_SOURCES} pngtest.c
	cd ${WRKSRC} && ${RM} -f ${source:R}.o && \
		${CC} ${CFLAGS} ${CPPFLAGS} ${OPENVMS_PNG_CPPFLAGS} \
		-c ${source} -o ${source:R}.o
.endfor
	cd ${WRKSRC} && ${RM} -f libpng16.a && \
		${AR} cr libpng16.a ${OPENVMS_PNG_SOURCES:C/\.c$/.o/}
	cd ${WRKSRC} && ${CC} ${CFLAGS} ${LDFLAGS} -o pngtest \
		pngtest.o libpng16.a ${BUILDLINK_DIR}/lib/libz.a
	${SED} -e 's|@prefix@|${PREFIX}|g' \
		-e 's|@exec_prefix@|${PREFIX}|g' \
		-e 's|@libdir@|${PREFIX}/lib|g' \
		-e 's|@includedir@|${PREFIX}/include|g' \
		-e 's|@PNGLIB_MAJOR@|1|g' -e 's|@PNGLIB_MINOR@|6|g' \
		-e 's|@PNGLIB_VERSION@|${PKGVERSION_NOREV}|g' \
		-e 's|@LIBS@|-lz -lm|g' \
		${WRKSRC}/libpng.pc.in > ${WRKSRC}/libpng16.pc
	${CP} ${WRKSRC}/libpng16.pc ${WRKSRC}/libpng.pc
	${SED} -e 's|@prefix@|${PREFIX}|g' \
		-e 's|@exec_prefix@|${PREFIX}|g' \
		-e 's|@libdir@|${PREFIX}/lib|g' \
		-e 's|@includedir@|${PREFIX}/include|g' \
		-e 's|@PNGLIB_MAJOR@|1|g' -e 's|@PNGLIB_MINOR@|6|g' \
		-e 's|@PNGLIB_VERSION@|${PKGVERSION_NOREV}|g' \
		-e 's|@LIBS@|-lz -lm|g' \
		${WRKSRC}/libpng-config.in > ${WRKSRC}/libpng16-config

do-test:
	cd ${WRKSRC} && ./pngtest

do-install:
	${INSTALL_SCRIPT} ${WRKSRC}/libpng16-config \
		${DESTDIR}${PREFIX}/bin/libpng16-config
	${INSTALL_SCRIPT} ${WRKSRC}/libpng16-config \
		${DESTDIR}${PREFIX}/bin/libpng-config
.for header in png.h pngconf.h pnglibconf.h
	${INSTALL_DATA} ${WRKSRC}/${header} ${DESTDIR}${PREFIX}/include
	${INSTALL_DATA} ${WRKSRC}/${header} \
		${DESTDIR}${PREFIX}/include/libpng16
.endfor
	${INSTALL_DATA} ${WRKSRC}/libpng16.a ${DESTDIR}${PREFIX}/lib
	${INSTALL_DATA} ${WRKSRC}/libpng.pc ${WRKSRC}/libpng16.pc \
		${DESTDIR}${PREFIX}/lib/pkgconfig
	${INSTALL_MAN} ${WRKSRC}/libpng.3 ${WRKSRC}/libpngpf.3 \
		${DESTDIR}${PREFIX}/${PKGMANDIR}/man3
	${INSTALL_MAN} ${WRKSRC}/png.5 \
		${DESTDIR}${PREFIX}/${PKGMANDIR}/man5
.else
PLIST.config=		yes
PLIST.libtool=		yes
PLIST.tools=		yes
.endif

# keep this in sync with the same code in buildlink3.mk
.if ${MACHINE_ARCH} != "i386" && ${MACHINE_ARCH} != "x86_64"
CPPFLAGS+=	-DPNG_NO_ASSEMBLER_CODE
.endif

SUBST_CLASSES+=		rpathfix
SUBST_FILES.rpathfix=	libpng-config.in
SUBST_VARS.rpathfix=	COMPILER_RPATH_FLAG
SUBST_STAGE.rpathfix=	post-build

.include "../../devel/zlib/buildlink3.mk"
.include "../../mk/bsd.pkg.mk"
