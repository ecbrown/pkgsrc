# $NetBSD: Makefile,v 1.12 2026/04/20 16:13:30 vins Exp $

DISTNAME=	ximaging-src-1.9.1
PKGNAME=	${DISTNAME:S,-src,,}
CATEGORIES=	graphics
MASTER_SITES=	https://fastestcode.org/dl/
EXTRACT_SUFX=	.tar.xz

MAINTAINER=	vins@NetBSD.org
HOMEPAGE=	https://fastestcode.org/emwm.html
COMMENT=	Image browser and viewer for Unix - X/Motif
LICENSE=	mit

USE_LANGUAGES=	c
PTHREAD_OPTS+=		require
PTHREAD_AUTO_VARS=	yes

.include "../../mk/bsd.prefs.mk"

.if ${OPSYS} == "OpenVMS"
# DECwindows supplies Motif, Xt, and Xlib as native shareable images.  Build
# one static object library and name those shareable images explicitly when
# linking, as the GNV linker cannot discover them through Unix -l options.
NO_CONFIGURE=		yes
JPEG_DEFAULT=		jpeg
JPEG_ACCEPTED=		jpeg

SUBST_CLASSES+=		openvms-motif
SUBST_STAGE.openvms-motif=	post-patch
SUBST_FILES.openvms-motif=	src/pathw.c src/toolbar.c
SUBST_SED.openvms-motif+=	-e 's/xmdpy->display.enable_thin_thickness/False/'
SUBST_SED.openvms-motif+=	-e 's/Dimension thin_borders;/Dimension thin_borders = 0;/'

OPENVMS_XIMAGING_LIB_SOURCES=	menu.c imgfile.c browser.c viewer.c \
	cmap.c pathw.c cursor.c imgblt.c pixconv.c comdlgs.c filemgmt.c \
	hashtbl.c defaults.c guiutil.c toolbar.c extres.c exec.c sgimage.c \
	sunras.c pbrush.c targa.c msbitmap.c xbitmap.c xpixmap.c netpbm.c \
	debug.c jpeg.c png.c tiff.c xipc.c openvms-fnmatch.c \
	openvms-motif.c
OPENVMS_XIMAGING_SOURCES=	main.c ${OPENVMS_XIMAGING_LIB_SOURCES}
OPENVMS_XIMAGING_CPPFLAGS=	-include openvms-motif.h \
	-DVMS -DXFree=Xfree -DENABLE_JPEG \
	-DENABLE_PNG -DENABLE_TIFF -I. -I/DECW$$INCLUDE \
	-I${BUILDLINK_DIR}/include
OPENVMS_XIMAGING_CFLAGS=	-O2 -Wc/PREFIX=ALL_ENTRIES

INSTALLATION_DIRS+=	bin lib/X11/app-defaults ${PKGMANDIR}/man1

post-patch:
	${CP} ${FILESDIR}/openvms-fnmatch.c ${WRKSRC}/src
	${CP} ${FILESDIR}/fnmatch.h ${WRKSRC}/src
	${CP} ${FILESDIR}/openvms-motif.c ${WRKSRC}/src
	${CP} ${FILESDIR}/openvms-motif.h ${WRKSRC}/src
	${MKDIR} ${WRKSRC}/src/Xm ${WRKSRC}/src/X11
	${CP} ${FILESDIR}/Container.h ${WRKSRC}/src/Xm
	${CP} ${FILESDIR}/ImUtil.h ${WRKSRC}/src/X11

do-build:
.for source in ${OPENVMS_XIMAGING_SOURCES}
	cd ${WRKSRC}/src && ${RM} -f ${source:R}.o && \
		/bin/dcl 'gcc ${OPENVMS_XIMAGING_CPPFLAGS} ${OPENVMS_XIMAGING_CFLAGS} -c -o ${source:R}.o ${source}' && \
		${TEST} -f ${source:R}.o
.endfor
	cd ${WRKSRC}/src && ${RM} -f libximaging.a ximaging ximaging.exe
.for source in ${OPENVMS_XIMAGING_LIB_SOURCES}
	cd ${WRKSRC}/src && /bin/ar.exe cr libximaging.a ${source:R}.o
.endfor
	${CP} ${BUILDLINK_DIR}/lib/libtiff.a ${WRKSRC}/src/libtiff.a
	${CP} ${BUILDLINK_DIR}/lib/libpng16.a ${WRKSRC}/src/libpng16.a
	${CP} ${BUILDLINK_DIR}/lib/libjpeg.a ${WRKSRC}/src/libjpeg.a
	${CP} ${BUILDLINK_DIR}/lib/libz.a ${WRKSRC}/src/libz.a
	${CP} ${FILESDIR}/openvms-x11.opt ${WRKSRC}/src/ximaging.opt
	cd ${WRKSRC}/src && /bin/dcl \
		'link/executable=ximaging.exe main.o,libximaging.a/library,libtiff.a/library,libpng16.a/library,libjpeg.a/library,libz.a/library,ximaging.opt/options' || \
		${TEST} -f ${WRKSRC}/src/ximaging.exe
	cd ${WRKSRC}/src && /bin/ln.exe ximaging.exe ximaging

do-install:
	${INSTALL_PROGRAM} ${WRKSRC}/src/ximaging ${DESTDIR}${PREFIX}/bin
	${INSTALL_DATA} ${WRKSRC}/src/XImaging.ad \
		${DESTDIR}${PREFIX}/lib/X11/app-defaults/XImaging
	${INSTALL_MAN} ${WRKSRC}/src/ximaging.1 \
		${DESTDIR}${PREFIX}/${PKGMANDIR}/man1
.else
SUBST_CLASSES+=		pkgsrc
SUBST_STAGE.pkgsrc=	pre-configure
SUBST_FILES.pkgsrc=	mf/Makefile.pkgsrc
SUBST_VARS.pkgsrc+=	PREFIX PKGMANDIR RCDIR
SUBST_MESSAGE.pkgsrc=	Replacing pkgsrc placeholders

# Enable OpenMP loop optimizations
MAKE_FLAGS+=	ENABLE_OMP=1
.endif

.include "../../graphics/png/buildlink3.mk"
.include "../../graphics/tiff/buildlink3.mk"
.if ${OPSYS} != "OpenVMS"
.include "../../x11/libXt/buildlink3.mk"
.include "../../x11/libXinerama/buildlink3.mk"
.include "../../mk/motif.buildlink3.mk"
.endif
.include "../../mk/pthread.buildlink3.mk"
.include "../../mk/jpeg.buildlink3.mk"
.include "../../mk/bsd.pkg.mk"
