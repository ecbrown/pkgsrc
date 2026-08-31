# $NetBSD: Makefile,v 1.41 2026/08/24 18:35:28 drixter Exp $

DISTNAME=	gzip-1.14
PKGREVISION=	1
CATEGORIES=	archivers
# does not provide uncompressed tarball, needed due to bootstrapping
#MASTER_SITES=	${MASTER_SITE_GNU:=gzip/}
MASTER_SITES=	${MASTER_SITE_LOCAL}
EXTRACT_SUFX=	.tar

MAINTAINER=	drixter@e-utp.net
HOMEPAGE=	https://www.gnu.org/software/gzip/gzip.html
COMMENT=	Compress or expand files
LICENSE=	gnu-gpl-v3

CONFLICTS+=		gzip-base-[0-9]* gzip-info-[0-9]* zutils-[0-9]*

GNU_CONFIGURE=	yes
CONFIGURE_ARGS.OpenVMS+=	--disable-dependency-tracking
MAKE_FLAGS+=	manlinks=so
INFO_FILES=	yes
CHECK_PERMS=	no	# can't depend on checkperms; its distfile is gzipped

.include "../../mk/bsd.prefs.mk"

.if ${OPSYS} == "OpenVMS"
# OpenVMS rename(2) rejects a read-only source file.  These generated files
# are made read-only immediately before they are renamed into place.
SUBST_CLASSES+=		gzip-vms-rename
SUBST_STAGE.gzip-vms-rename=	pre-configure
SUBST_FILES.gzip-vms-rename=	Makefile.in
SUBST_SED.gzip-vms-rename=	-e 's,chmod a-w [$$]@t,chmod u+w $$@t,'
SUBST_SED.gzip-vms-rename+=	-e 's|chmod a=rx $$@-t|chmod u=rwx,go=rx $$@-t|'

# VSI C cannot reopen the ODS-5 name float+.h from a quoted #include.
SUBST_CLASSES+=		gzip-vms-float-header
SUBST_STAGE.gzip-vms-float-header=	pre-configure
SUBST_FILES.gzip-vms-float-header=	lib/isnan.c lib/signbitd.c
SUBST_FILES.gzip-vms-float-header+=	lib/signbitf.c lib/signbitl.c
SUBST_FILES.gzip-vms-float-header+=	lib/vasnprintf.c
SUBST_SED.gzip-vms-float-header=	-e 's,"float+.h","floatplus.h",g'

post-patch:
	${CP} ${WRKSRC}/lib/float+.h ${WRKSRC}/lib/floatplus.h
.endif

.include "../../mk/bsd.pkg.mk"
