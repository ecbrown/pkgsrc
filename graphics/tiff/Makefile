# $NetBSD: Makefile,v 1.174 2026/07/03 11:41:35 gdt Exp $

VERSION_BASE=	4.7.2
# Accomodate testing unreleased versions.
#VERSION_RC=	rc4
DISTNAME=	tiff-${VERSION_BASE}${VERSION_RC}
CATEGORIES=	graphics
MASTER_SITES=	https://download.osgeo.org/libtiff/
EXTRACT_SUFX=	.tar.xz

MAINTAINER=	gdt@NetBSD.org
HOMEPAGE=	http://simplesystems.org/libtiff/
COMMENT=	Library and tools for reading and writing TIFF data files
LICENSE=	mit

WRKSRC=		${WRKDIR}/tiff-${VERSION_BASE}

EXTRACT_ONLY=	${DISTNAME}${EXTRACT_SUFX}

USE_LANGUAGES=		c
USE_CC_FEATURES=	c99
USE_TOOLS=		gmake pkg-config
USE_LIBTOOL=		yes
GNU_CONFIGURE=		yes

CONFIGURE_ARGS+=	--enable-rpath
CONFIGURE_ARGS+=	--with-docdir=${PREFIX}/share/doc/tiff
CONFIGURE_ARGS+=	--disable-libdeflate
CONFIGURE_ARGS+=	--disable-sphinx
CONFIGURE_ARGS+=	--disable-webp
CONFIGURE_ARGS+=	--disable-zstd
CONFIGURE_ARGS+=	--disable-opengl

.include "../../mk/bsd.prefs.mk"

.if ${OPSYS} == "OpenVMS"
# Keep the initial OpenVMS library small and static.  These optional codecs
# pull in libraries that have not been ported yet; JPEG and Deflate cover the
# formats needed by the Motif XImaging viewer.  GNU make's Automake recursion
# cannot reliably exec the nested GNV shell on OpenVMS, so compile the static
# object library directly with an explicit platform configuration.
.undef USE_LIBTOOL
.undef GNU_CONFIGURE
NO_CONFIGURE=		yes
JPEG_DEFAULT=		jpeg
JPEG_ACCEPTED=		jpeg
PLIST_SRC=		${PKGDIR}/PLIST.OpenVMS

OPENVMS_TIFF_SOURCES=	tif_aux.c tif_close.c tif_codec.c tif_color.c \
	tif_compress.c tif_dir.c tif_dirinfo.c tif_dirread.c tif_dirwrite.c \
	tif_dumpmode.c tif_error.c tif_extension.c tif_fax3.c tif_fax3sm.c \
	tif_flush.c tif_getimage.c tif_hash_set.c tif_jbig.c tif_jpeg.c \
	tif_jpeg_12.c tif_lerc.c tif_luv.c tif_lzma.c tif_lzw.c tif_next.c \
	tif_ojpeg.c tif_open.c tif_packbits.c tif_pixarlog.c tif_predict.c \
	tif_print.c tif_read.c tif_strip.c tif_swab.c tif_thunder.c \
	tif_tile.c tif_version.c tif_warning.c tif_webp.c tif_write.c \
	tif_zip.c tif_zstd.c tif_unix.c
# VSI C does not provide GCC's SSE2 intrinsic header despite advertising the
# x86-64 architecture; use libtiff's portable predictor implementation.
OPENVMS_TIFF_CPPFLAGS=	-U__x86_64__ -DHAVE_CONFIG_H -I. \
	-I${BUILDLINK_DIR}/include
OPENVMS_TIFF_CFLAGS=	-O2 -Wc/PREFIX=ALL_ENTRIES

INSTALLATION_DIRS+=	include lib lib/pkgconfig

post-extract:
	${CP} ${FILESDIR}/openvms-tif_config.h \
		${WRKSRC}/libtiff/tif_config.h
	${CP} ${FILESDIR}/openvms-tiffconf.h \
		${WRKSRC}/libtiff/tiffconf.h

do-build:
.for source in ${OPENVMS_TIFF_SOURCES}
	cd ${WRKSRC}/libtiff && ${RM} -f ${source:R}.o && \
		/bin/dcl 'gcc ${OPENVMS_TIFF_CPPFLAGS} ${OPENVMS_TIFF_CFLAGS} -c -o ${source:R}.o ${source}' && \
		${TEST} -f ${source:R}.o
.endfor
	cd ${WRKSRC}/libtiff && ${RM} -f libtiff.a
.for source in ${OPENVMS_TIFF_SOURCES}
	cd ${WRKSRC}/libtiff && /bin/ar.exe cr libtiff.a ${source:R}.o
.endfor
	${CP} ${FILESDIR}/openvms-test.c ${WRKSRC}/libtiff/openvms-test.c
	${CP} ${BUILDLINK_DIR}/lib/libjpeg.a ${WRKSRC}/libtiff/libjpeg.a
	${CP} ${BUILDLINK_DIR}/lib/libz.a ${WRKSRC}/libtiff/libz.a
	cd ${WRKSRC}/libtiff && /bin/dcl \
		'gcc ${OPENVMS_TIFF_CPPFLAGS} ${OPENVMS_TIFF_CFLAGS} -c -o openvms-test.o openvms-test.c' && \
		${TEST} -f openvms-test.o
	cd ${WRKSRC}/libtiff && /bin/dcl \
		'link/executable=openvms-test.exe openvms-test.o,libtiff.a/library,libjpeg.a/library,libz.a/library' || \
		${TEST} -f ${WRKSRC}/libtiff/openvms-test.exe
	cd ${WRKSRC}/libtiff && /bin/ln.exe openvms-test.exe openvms-test
	${SED} -e 's|@prefix@|${PREFIX}|g' \
		-e 's|@exec_prefix@|$${prefix}|g' \
		-e 's|@libdir@|$${exec_prefix}/lib|g' \
		-e 's|@includedir@|$${prefix}/include|g' \
		-e 's|@LIBTIFF_MAJOR_VERSION@|4|g' \
		-e 's|@LIBTIFF_MINOR_VERSION@|7|g' \
		-e 's|@LIBTIFF_MICRO_VERSION@|2|g' \
		-e 's|@tiff_libs_private@|-ljpeg -lz -lm|g' \
		-e 's|@tiff_requires_private@|libjpeg zlib|g' \
		${WRKSRC}/libtiff-4.pc.in > ${WRKSRC}/libtiff-4.pc

do-test:
	cd ${WRKSRC}/libtiff && ./openvms-test

do-install:
	${INSTALL_DATA} ${WRKSRC}/libtiff/tiff.h \
		${WRKSRC}/libtiff/tiffconf.h ${WRKSRC}/libtiff/tiffio.h \
		${WRKSRC}/libtiff/tiffvers.h ${DESTDIR}${PREFIX}/include
	${INSTALL_DATA} ${WRKSRC}/libtiff/libtiff.a \
		${DESTDIR}${PREFIX}/lib
	${INSTALL_DATA} ${WRKSRC}/libtiff-4.pc \
		${DESTDIR}${PREFIX}/lib/pkgconfig
.else
USE_LANGUAGES+=		c++
CONFIGURE_ARGS+=	--enable-cxx
CONFIGURE_ARGS+=	--enable-lerc
.endif

PKGCONFIG_OVERRIDE=	libtiff-4.pc.in

# NetBSD 10 amd64: pass except 4 cases, because it uses non-POSIX diff -Z
# NetBSD 11.99.2/amd64: all tests pass
TEST_TARGET=		check

.if ${OPSYS} != "OpenVMS"
post-install:
	cd ${DESTDIR}${PREFIX}/${PKGMANDIR}/man3 && for file in *.3tiff; do\
		target=`${ECHO} $$file | sed "s/3tiff/3/"`; \
		${MV} $$file $$target; \
	done
.endif

.include "options.mk"

.include "../../devel/zlib/buildlink3.mk"
.if ${OPSYS} != "OpenVMS"
# XXX for liblzma; there are more potential suppliers
.include "../../archivers/xz/buildlink3.mk"
.include "../../graphics/jbigkit/buildlink3.mk"
.include "../../graphics/lerc/buildlink3.mk"
.endif
.include "../../mk/jpeg.buildlink3.mk"
.include "../../mk/bsd.pkg.mk"
