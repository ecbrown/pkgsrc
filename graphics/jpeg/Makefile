# $NetBSD: Makefile,v 1.60 2025/05/07 09:13:54 jperkin Exp $

DISTNAME=	jpegsrc.v9f
PKGNAME=	${DISTNAME:S/src.v/-/}
PKGREVISION=	1
CATEGORIES=	graphics
MASTER_SITES=	http://www.ijg.org/files/

MAINTAINER=	pkgsrc-users@NetBSD.org
HOMEPAGE=	https://www.ijg.org/
COMMENT=	IJG's jpeg compression utilities
LICENSE=	ijg

CONFLICTS=		libjpeg-turbo-[0-9]*

WRKSRC=			${WRKDIR}/${PKGNAME_NOREV}

.include "../../mk/bsd.prefs.mk"

.if ${OPSYS} == "OpenVMS"
# The distribution still ships a native VMS configuration.  Build the
# portable static library directly; GNU libtool does not support OpenVMS
# shareable images.
NO_CONFIGURE=		yes
PLIST.static=		yes

OPENVMS_JPEG_LIB_SOURCES=	jaricom.c jcapimin.c jcapistd.c jcarith.c \
	jctrans.c jcparam.c jdatadst.c jcinit.c jcmaster.c jcmarker.c \
	jcmainct.c jcprepct.c jccoefct.c jccolor.c jcsample.c jchuff.c \
	jcdctmgr.c jfdctfst.c jfdctflt.c jfdctint.c jdapimin.c jdapistd.c \
	jdarith.c jdtrans.c jdatasrc.c jdmaster.c jdinput.c jdmarker.c \
	jdhuff.c jdmainct.c jdcoefct.c jdpostct.c jddctmgr.c jidctfst.c \
	jidctflt.c jidctint.c jdsample.c jdcolor.c jquant1.c jquant2.c \
	jdmerge.c jcomapi.c jutils.c jerror.c jmemmgr.c jmemnobs.c
OPENVMS_JPEG_APP_SOURCES=	cjpeg.c rdppm.c rdgif.c rdtarga.c rdrle.c \
	rdbmp.c rdswitch.c cdjpeg.c djpeg.c wrppm.c wrgif.c wrtarga.c \
	wrrle.c wrbmp.c rdcolmap.c jpegtran.c transupp.c rdjpgcom.c \
	wrjpgcom.c jpegexiforient.c

OPENVMS_JPEG_CJPEG_OBJECTS=	cjpeg.o rdppm.o rdgif.o rdtarga.o rdrle.o \
	rdbmp.o rdswitch.o cdjpeg.o
OPENVMS_JPEG_DJPEG_OBJECTS=	djpeg.o wrppm.o wrgif.o wrtarga.o wrrle.o \
	wrbmp.o rdcolmap.o cdjpeg.o
OPENVMS_JPEG_TRAN_OBJECTS=	jpegtran.o rdswitch.o cdjpeg.o transupp.o

INSTALLATION_DIRS=	bin include lib lib/pkgconfig ${PKGMANDIR}/man1 \
	share/doc/jpeg

do-build:
.for source in ${OPENVMS_JPEG_LIB_SOURCES} ${OPENVMS_JPEG_APP_SOURCES}
	cd ${WRKSRC} && ${RM} -f ${source:R}.o && \
		${CC} ${CFLAGS} ${CPPFLAGS} -c ${source} -o ${source:R}.o
.endfor
	cd ${WRKSRC} && ${RM} -f libjpeg.a && \
		${AR} cr libjpeg.a ${OPENVMS_JPEG_LIB_SOURCES:C/\.c$/.o/}
	cd ${WRKSRC} && ${CC} ${CFLAGS} ${LDFLAGS} -o cjpeg \
		${OPENVMS_JPEG_CJPEG_OBJECTS} libjpeg.a
	cd ${WRKSRC} && ${CC} ${CFLAGS} ${LDFLAGS} -o djpeg \
		${OPENVMS_JPEG_DJPEG_OBJECTS} libjpeg.a
	cd ${WRKSRC} && ${CC} ${CFLAGS} ${LDFLAGS} -o jpegtran \
		${OPENVMS_JPEG_TRAN_OBJECTS} libjpeg.a
.for program in rdjpgcom wrjpgcom jpegexiforient
	cd ${WRKSRC} && ${CC} ${CFLAGS} ${LDFLAGS} -o ${program} \
		${program}.o
.endfor
	${SED} -e 's|@prefix@|${PREFIX}|g' \
		-e 's|@exec_prefix@|${PREFIX}|g' \
		-e 's|@libdir@|${PREFIX}/lib|g' \
		-e 's|@includedir@|${PREFIX}/include|g' \
		-e 's|@JPEG_LIB_VERSION_MAJOR@|9|g' \
		-e 's|@JPEG_LIB_VERSION_MINOR@|6|g' \
		${WRKSRC}/libjpeg.pc.in > ${WRKSRC}/libjpeg.pc

do-install:
.for program in cjpeg djpeg jpegtran rdjpgcom wrjpgcom jpegexiforient
	${INSTALL_PROGRAM} ${WRKSRC}/${program} ${DESTDIR}${PREFIX}/bin
.endfor
	${INSTALL_DATA} ${WRKSRC}/jconfig.h ${WRKSRC}/jerror.h \
		${WRKSRC}/jmorecfg.h ${WRKSRC}/jpeglib.h \
		${DESTDIR}${PREFIX}/include
	${INSTALL_DATA} ${WRKSRC}/libjpeg.a ${DESTDIR}${PREFIX}/lib
	${INSTALL_DATA} ${WRKSRC}/libjpeg.pc \
		${DESTDIR}${PREFIX}/lib/pkgconfig
.for manpage in cjpeg djpeg jpegtran rdjpgcom wrjpgcom
	${INSTALL_MAN} ${WRKSRC}/${manpage}.1 \
		${DESTDIR}${PREFIX}/${PKGMANDIR}/man1
.endfor
.else
USE_LIBTOOL=		yes
GNU_CONFIGURE=		yes
CONFIGURE_ARGS+=	--disable-ld-version-script
PLIST.libtool=		yes

INSTALLATION_DIRS=	bin include share/doc/jpeg

PKGCONFIG_OVERRIDE=	libjpeg.pc.in
TEST_TARGET=		test
.endif
PLIST_VARS+=		libtool static

.if ${JPEG_DEFAULT:U} == "jpeg"
SUPERSEDES+=	libjpeg-turbo-[0-9]*
.endif

BUILD_DEFS+=		JPEG_DEFAULT

post-extract:
	${CP} ${FILESDIR}/jpegexiforient.c ${WRKSRC}
.if ${OPSYS} == "OpenVMS"
	${CP} ${WRKSRC}/jconfig.vms ${WRKSRC}/jconfig.h
.endif

post-install:
	${INSTALL_SCRIPT} ${FILESDIR}/exifautotran ${DESTDIR}${PREFIX}/bin
	${INSTALL_DATA} ${WRKSRC}/jpegint.h ${DESTDIR}${PREFIX}/include
	${INSTALL_DATA} ${WRKSRC}/*.txt ${DESTDIR}${PREFIX}/share/doc/jpeg

.include "../../mk/bsd.pkg.mk"
