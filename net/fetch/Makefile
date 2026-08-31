# $NetBSD: Makefile,v 1.24 2023/10/24 22:10:15 wiz Exp $

DISTNAME=	fetch-1.9
PKGREVISION=	2
CATEGORIES=	net
MASTER_SITES=	# empty
DISTFILES=	# empty

MAINTAINER=	pkgsrc-users@NetBSD.org
HOMEPAGE=	https://www.FreeBSD.org/
COMMENT=	Client to fetch URLs
LICENSE=	modified-bsd

USE_FEATURES=		nbcompat

GNU_CONFIGURE=		yes
USE_BSD_MAKEFILE=	yes

.include "../../mk/bsd.prefs.mk"
.if !empty(FETCH_USING:Mfetch)
CHECK_PERMS=		no
.endif
.if ${OPSYS} == "OpenVMS"
# Socket and resolver routines are part of the C RTL.  The VSI compiler
# driver only warns for missing Unix-style libraries, which would otherwise
# make configure record false-positive -lsocket and -lnsl dependencies.
CONFIGURE_ENV+=		ac_cv_lib_socket_socket=no
CONFIGURE_ENV+=		ac_cv_lib_nsl_gethostbyname=no
.endif

INSTALLATION_DIRS=	bin ${PKGMANDIR}/cat1 ${PKGMANDIR}/man1

do-extract:
	${CP} -r ${FILESDIR} ${WRKSRC}

.include "../../net/libfetch/buildlink3.mk"
.include "../../mk/bsd.pkg.mk"
