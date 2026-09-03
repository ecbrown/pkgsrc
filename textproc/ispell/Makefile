# $NetBSD: Makefile,v 1.32 2025/04/21 21:26:46 wiz Exp $

.include	"Makefile.common"

PKGREVISION=	3
CATEGORIES=	textproc
MASTER_SITES=	https://www.cs.hmc.edu/~geoff/tars/

MAINTAINER=	pkgsrc-users@NetBSD.org
HOMEPAGE=	https://www.cs.hmc.edu/~geoff/ispell.html
COMMENT=	Interactive spelling checker

CONFLICTS+=		ispell-base-[0-9]*
FORCE_C_STD=		gnu99
USE_TOOLS+=		gmake grep:run mktemp:run yacc
INSTALLATION_DIRS=	bin lib ${PKGMANDIR}/man1 ${PKGMANDIR}/man5

MAKE_FLAGS+=	TMPDIR=${WRKDIR:Q}
MAKE_JOBS_SAFE=	no

.include "../../mk/bsd.prefs.mk"

PLIST_VARS+=	vms
.if ${OPSYS} == "OpenVMS"
PLIST.vms=	yes
INSTALLATION_DIRS+=	startup
CFLAGS+=	-D_POSIX_EXIT=1 -D_USE_STD_STAT=1 -Wc/MAIN=POSIX_EXIT
MAKE_FLAGS+=	STRIP=${TRUE:Q}
.else
MAKE_FLAGS+=	STRIP=${STRIP:Q}
.endif

.if exists(/usr/include/inttypes.h) && empty(MACHINE_PLATFORM:MIRIX-5.3-*)
CFLAGS+=	-DHAVE_INTTYPES_H
.endif

.if ${OPSYS} == "SunOS"
EXTRADICT=	/usr/dict/words
.else
EXTRADICT=	/usr/share/dict/words
.endif

.if !exists(${EXTRADICT})
EXTRADICT=
.endif
.if empty(EXTRADICT)
ISPELL_WORDS=	/dev/null
.else
ISPELL_WORDS=	${EXTRADICT}
.endif

do-configure:
	cd ${WRKSRC};							\
	${SED} -e 's|/usr/local|${PREFIX}|g' <local.h.bsd >local.h;	\
	${ECHO} "#define LANGUAGES \"{american,MASTERDICTS=american.med+,HASHFILES=americanmed+.hash,EXTRADICT=${EXTRADICT}}\""  >>local.h; \
	${ECHO} "#define MASTERHASH \"americanmed+.hash\"" >>local.h;	\
	${ECHO} "#define MASKBITS 64" >>local.h;			\
	${ECHO} "#undef WORDS"  >> local.h ;				\
	${ECHO} '#define WORDS   "${ISPELL_WORDS}"'  >> local.h ;		\
	${ECHO} "#define CFLAGS \"${CFLAGS}\"" >> local.h ;		\
	${ECHO} "#define LIBES \"${LDFLAGS}\"" >> local.h ;		\
	${ECHO} "#define MAN1DIR \"${PREFIX}/${PKGMANDIR}/man1\"" >> local.h ; \
	${ECHO} "#define MAN45DIR \"${PREFIX}/${PKGMANDIR}/man5\"" >> local.h ;
.if ${OPSYS} == "OpenVMS"
	cd ${WRKSRC};							\
	${ECHO} '#define CC "${WRAPPER_DIR}/bin/cc"' >> local.h; \
	${ECHO} '#define EGREPCMD "grep -Ei"' >> local.h;		\
	${ECHO} "#define HAVE_INTTYPES_H 1" >> local.h;		\
	${ECHO} '#define TERMLIB "${BUILDLINK_LDADD.ncurses}"' >> local.h; \
	${ECHO} '#define BAKEXT "_BAK"' >> local.h;		\
	${ECHO} "#define USG 1" >> local.h;				\
	${ECHO} "#define USESH 1" >> local.h;				\
	${ECHO} '#define YACC "${TOOLS_CMD.yacc}"' >> local.h;	\
	${ECHO} '#define TEMPNAME "ispellXXXXXX"' >> local.h;
.endif
.if ${OPSYS} == "SunOS"
	cd ${WRKSRC};							\
	${ECHO} "#define USG" >> local.h;
.endif

.if ${OPSYS} == "OpenVMS"
# pkg_create cannot reliably archive the absolute symbolic links produced by
# the upstream language install.  Independent regular files also keep binary
# packages usable when extracted without OpenVMS link metadata.
post-install:
	${RM} -f ${DESTDIR}${PREFIX}/lib/american.hash \
		${DESTDIR}${PREFIX}/lib/english.hash
	${INSTALL_DATA} ${DESTDIR}${PREFIX}/lib/americanmed+.hash \
		${DESTDIR}${PREFIX}/lib/american.hash
	${INSTALL_DATA} ${DESTDIR}${PREFIX}/lib/americanmed+.hash \
		${DESTDIR}${PREFIX}/lib/english.hash
	${INSTALL_SCRIPT} ${FILESDIR}/ISPELL_SETUP.COM \
		${DESTDIR}${PREFIX}/startup/ISPELL_SETUP.COM
.endif

.if ${OPSYS} == "OpenVMS"
# The native keystroke shim is supplied by pkgsrc's static ncurses library.
BUILDLINK_API_DEPENDS.ncurses+=	ncurses>=6.6nb1
.include "../../devel/ncurses/buildlink3.mk"
.else
.include "../../mk/termcap.buildlink3.mk"
.endif
.include "../../mk/bsd.pkg.mk"
