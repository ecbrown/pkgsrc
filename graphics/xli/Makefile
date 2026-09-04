# $NetBSD: Makefile,v 1.48 2025/04/24 14:14:42 wiz Exp $

DISTNAME=	xli-2005-02-27
PKGNAME=	xli-1.17.0
PKGREVISION=	15
CATEGORIES=	graphics x11
#MASTER_SITES=	http://pantransit.reptiles.org/prog/xli/

MAINTAINER=	pkgsrc-users@NetBSD.org
#HOMEPAGE=	http://pantransit.reptiles.org/prog/
COMMENT=	X11 Image Loading Utility

.include "../../mk/bsd.prefs.mk"

PLIST_VARS+=		imake openvms

.if ${OPSYS} == "OpenVMS"
# DECwindows supplies Xlib as a native shareable image rather than a pkgsrc
# library.  Build the portable image loaders as one static object library and
# link it with that shareable image explicitly.
NO_CONFIGURE=		yes
JPEG_DEFAULT=		jpeg
JPEG_ACCEPTED=		jpeg
PLIST.openvms=		yes

OPENVMS_XLI_SOURCES=	bright.c clip.c cmuwmrast.c compress.c dither.c \
	faces.c fbm.c fill.c g3.c gif.c halftone.c imagetypes.c img.c \
	mac.c mcidas.c mc_tables.c merge.c misc.c new.c options.c path.c \
	pbm.c pcx.c reduce.c jpeg.c rle.c rlelib.c root.c rotate.c send.c \
	smooth.c sunraster.c value.c window.c xbitmap.c xpixmap.c xwd.c \
	zio.c zoom.c ddxli.c tga.c bmp.c pcd.c png.c
# DECwindows exports the historical Xfree spelling while its current header
# declares XFree.  Preserve the source API and map that one external symbol.
OPENVMS_XLI_CPPFLAGS=	-DVMS -DNO_XSHM -DHAS_MEMCPY \
	-DXFree=Xfree -I/DECW$$INCLUDE -I${BUILDLINK_DIR}/include
OPENVMS_XLI_CFLAGS=	-O2 -Wc/PREFIX=ALL_ENTRIES

SUBST_CLASSES+=		openvms-prefix
SUBST_STAGE.openvms-prefix=	pre-build
SUBST_FILES.openvms-prefix=	path.c
SUBST_SED.openvms-prefix=	-e 's|@PREFIX@|${PREFIX}|g'

INSTALLATION_DIRS+=	bin lib/X11/app-defaults ${PKGMANDIR}/man1

post-patch:
	${CP} ${WRKSRC}/patchlevel ${WRKSRC}/patchlevel.h

do-build:
.for source in ${OPENVMS_XLI_SOURCES} xli.c xlito.c
	cd ${WRKSRC} && ${RM} -f ${source:R}.o && \
		/bin/dcl 'gcc ${OPENVMS_XLI_CPPFLAGS} ${OPENVMS_XLI_CFLAGS} -c -o ${source:R}.o ${source}' && \
		${TEST} -f ${source:R}.o
.endfor
	cd ${WRKSRC} && ${RM} -f libxli.a xli xli.exe xlito xlito.exe
.for source in ${OPENVMS_XLI_SOURCES}
	cd ${WRKSRC} && /bin/ar.exe cr libxli.a ${source:R}.o
.endfor
	${CP} ${BUILDLINK_DIR}/lib/libjpeg.a ${WRKSRC}/libjpeg.a
	${CP} ${BUILDLINK_DIR}/lib/libpng16.a ${WRKSRC}/libpng16.a
	${CP} ${BUILDLINK_DIR}/lib/libz.a ${WRKSRC}/libz.a
	${CP} ${FILESDIR}/openvms-x11.opt ${WRKSRC}/xli.opt
	cd ${WRKSRC} && /bin/dcl \
		'link/executable=xli.exe xli.o,libxli.a/library,libjpeg.a/library,libpng16.a/library,libz.a/library,xli.opt/options' || \
		${TEST} -f ${WRKSRC}/xli.exe
	cd ${WRKSRC} && /bin/ln.exe xli.exe xli
	cd ${WRKSRC} && /bin/dcl 'link/executable=xlito.exe xlito.o' || \
		${TEST} -f ${WRKSRC}/xlito.exe
	cd ${WRKSRC} && /bin/ln.exe xlito.exe xlito

do-test:
	cd ${WRKSRC} && ./xli -identify chkgamma.jpg

do-install:
	${INSTALL_PROGRAM} ${WRKSRC}/xli ${DESTDIR}${PREFIX}/bin/xli
	${INSTALL_PROGRAM} ${WRKSRC}/xli ${DESTDIR}${PREFIX}/bin/xsetbg
	${INSTALL_PROGRAM} ${WRKSRC}/xli ${DESTDIR}${PREFIX}/bin/xview
	${INSTALL_PROGRAM} ${WRKSRC}/xlito ${DESTDIR}${PREFIX}/bin/xlito
	${INSTALL_DATA} ${FILESDIR}/Xli \
		${DESTDIR}${PREFIX}/lib/X11/app-defaults/Xli
	${INSTALL_MAN} ${WRKSRC}/xli.man \
		${DESTDIR}${PREFIX}/${PKGMANDIR}/man1/xli.1
	${INSTALL_MAN} ${WRKSRC}/xlito.man \
		${DESTDIR}${PREFIX}/${PKGMANDIR}/man1/xlito.1
.else
USE_IMAKE=		YES
PLIST.imake=		yes
INSTALLATION_DIRS+=	lib/X11/app-defaults
.endif

.include "../../mk/jpeg.buildlink3.mk"
.include "../../graphics/png/buildlink3.mk"
.if ${OPSYS} != "OpenVMS"
.include "../../x11/libX11/buildlink3.mk"
.include "../../x11/libXext/buildlink3.mk"
.endif
.include "../../mk/bsd.pkg.mk"
