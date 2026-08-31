# $NetBSD: Makefile,v 1.80 2026/07/25 18:15:50 morr Exp $

PKGNAME=		vim-share-${VIM_VERSION}.${VIM_PATCHLEVEL}
COMMENT=		Data files for the vim editor (vi clone)

.include		"Makefile.common"

CONFIGURE_ARGS+=	--disable-gui
CONFIGURE_ARGS+=	--without-x

.if ${OPSYS} == "OpenVMS"
USE_TOOLS+=		gmake
INSTALLATION_DIRS+=	bin

# The OpenVMS distribution has a separate native MMS build for xxd.  Copy
# the resulting image to the name expected by the Unix runtime installer;
# that installer is still useful for laying out Vim's portable data files.
do-build:
	${TOUCH} ${WRKSRC}/auto/config.mk
	cd ${WRKSRC}/xxd && \
		/bin/dcl 'MMS/DESCRIP=Make_vms.mms/IGNORE=WARNING'
	${CP} ${WRKSRC}/xxd/X86_64/xxd.exe ${WRKSRC}/xxd/xxd

do-install:
	cd ${WRKSRC} && ${SETENV} ${INSTALL_ENV} ${GMAKE} -f Makefile \
		DESTDIR=${DESTDIR:Q} prefix=${PREFIX:Q} \
		exec_prefix=${PREFIX:Q} BINDIR=${PREFIX:Q}/bin \
		DATADIR=${PREFIX:Q}/share MANDIR=${PREFIX:Q}/${PKGMANDIR} \
		VIMTARGET= CROSS_COMPILING=yes STRIP=/bin/true \
		installruntime installtools
.elif ${OPSYS} == "Cygwin"
BUILD_TARGET=		vim.exe
.else
BUILD_TARGET=		vim
.endif
.if ${OPSYS} != "OpenVMS"
INSTALL_TARGET=		installruntime installmacros installtutor installtools
.endif
USE_TOOLS+=		awk
INSTALL_MAKE_FLAGS+=	${MAKE_FLAGS} TOOLS_AWK=${TOOLS_AWK} TOOLS_SETENV=${TOOLS_SETENV}

PLIST_SUBST+=		VIM_SUBDIR=${VIM_SUBDIR}

CHECK_INTERPRETER_SKIP+=	share/vim/${VIM_SUBDIR}/doc/*.pl
CHECK_INTERPRETER_SKIP+=	share/vim/${VIM_SUBDIR}/tools/*.pl
CHECK_INTERPRETER_SKIP+=	share/vim/${VIM_SUBDIR}/tools/*.py
CHECK_INTERPRETER_SKIP+=	share/vim/${VIM_SUBDIR}/tools/*.awk

.if ${OPSYS} != "OpenVMS"
.include "../../mk/curses.buildlink3.mk"
.endif
.include "../../mk/bsd.pkg.mk"
