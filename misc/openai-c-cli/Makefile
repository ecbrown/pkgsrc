# $NetBSD$

DISTNAME=	openai-c-cli-0.1
CATEGORIES=	misc
MASTER_SITES=	# empty
DISTFILES=	# empty

MAINTAINER=	pkgsrc-users@NetBSD.org
HOMEPAGE=	https://developers.openai.com/api/docs/guides/migrate-to-responses
COMMENT=	Small C command-line client for the OpenAI Responses API
LICENSE=	2-clause-bsd

USE_LANGUAGES=	c

DEPENDS+=	mozilla-rootcerts>=1.1.20260715:../../security/mozilla-rootcerts
BUILDLINK_API_DEPENDS.openssl+=	openssl>=1.1.1

INSTALLATION_DIRS=	bin ${PKGMANDIR}/man1

SUBST_CLASSES+=	prefix
SUBST_STAGE.prefix=	pre-configure
SUBST_FILES.prefix=	openai-c.c
SUBST_VARS.prefix=	PREFIX

do-extract:
	${MKDIR} ${WRKSRC}
	${CP} ${FILESDIR}/openai-c.c ${FILESDIR}/openai-c.1 ${WRKSRC}

.include "../../mk/bsd.prefs.mk"

.if ${OPSYS} == "OpenVMS"
do-build:
	cd ${WRKSRC} && ${RM} -f openai.o openai openai.exe && \
		/bin/dcl 'gcc -O2 -Wc/PREFIX=ALL_ENTRIES -Wc/MAIN=POSIX_EXIT -c -o openai.o openai-c.c' && \
		${TEST} -f openai.o
	${CP} ${OPENSSL_OPENVMS_GNV_OPT} '${WRKSRC}/gnv$$openai.opt'
	cd ${WRKSRC} && /bin/dcl \
		'link/executable=openai.exe openai.o,gnv$$openai.opt/options' || \
		${TEST} -f ${WRKSRC}/openai.exe
.else
do-build:
	cd ${WRKSRC} && ${CC} ${CPPFLAGS} ${CFLAGS} -o openai \
		openai-c.c ${LDFLAGS} -lssl -lcrypto
.endif

do-install:
.if ${OPSYS} == "OpenVMS"
	${INSTALL_PROGRAM} ${WRKSRC}/openai.exe \
		${DESTDIR}${PREFIX}/bin/openai-c
.else
	${INSTALL_PROGRAM} ${WRKSRC}/openai \
		${DESTDIR}${PREFIX}/bin/openai-c
.endif
	${INSTALL_MAN} ${WRKSRC}/openai-c.1 \
		${DESTDIR}${PREFIX}/${PKGMANDIR}/man1/openai-c.1

.include "../../security/openssl/buildlink3.mk"
.include "../../mk/bsd.pkg.mk"
