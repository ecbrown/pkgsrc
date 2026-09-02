# $NetBSD: Makefile,v 1.10 2016/12/30 22:25:46 wiz Exp $

PKGNAME=	${DISTNAME:S/emacs/emacs22/:S/-/-nox11-/}
PKGREVISION=	10

CONFLICTS+=	emacs22-[0-9]*

FILESDIR=	${.CURDIR}/../../editors/emacs22/files
PATCHDIR=	${.CURDIR}/../../editors/emacs22/patches
PKGDIR=		${.CURDIR}/../../editors/emacs22

# This needs to be set before including bsd.prefs.mk.
SETGIDGAME=		yes

# The VMS build is selected by the shared makefile; keep this flavor
# terminal-only just as the historical Unix package did.
EMACS22_WITH_X=	NO

.include "../../editors/emacs22/Makefile.common"

# The native VMS install has no setgid score files.  The historical no-X
# setting above must not pull games-user/group tasks into the binary package.
.if ${OPSYS} == "OpenVMS"
SETGIDGAME=	no
USE_GAMESGROUP=	no
.endif

.include "../../mk/bsd.pkg.mk"
