# $NetBSD: Makefile,v 1.159 2026/05/14 16:42:22 ryoon Exp $

DISTNAME=	lynx2.9.2
PKGNAME=	${DISTNAME:S/lynx/lynx-/:S/rel//}
PKGREVISION=	9
CATEGORIES=	www
MASTER_SITES=	https://invisible-mirror.net/archives/lynx/tarballs/
EXTRACT_SUFX=	.tar.bz2

MAINTAINER=	pkgsrc-users@NetBSD.org
HOMEPAGE=	https://lynx.invisible-island.net/
COMMENT=	Alphanumeric display oriented World-Wide Web Client
LICENSE=	gnu-gpl-v2 # OpenSSL exception

# problem with WWW/Library/Implementation
MAKE_JOBS_SAFE=		no

SUBDIR=				current
USE_PKGLOCALEDIR=		yes
GNU_CONFIGURE=			yes
BROKEN_GETTEXT_DETECTION=	yes
INSTALL_TARGET=			install-full
MAKE_FILE=			makefile

CONFIGURE_ENV+=		GZIP=${TOOLS_GZIP_CMD:Q}
CONFIGURE_ENV+=		UNZIP=${TOOLS_PATH.unzip:Q}
USE_TOOLS+=		gzip unzip:run

PLIST_VARS+=		nls

CONF_FILES+=		share/examples/lynx/lynx.cfg ${PKG_SYSCONFDIR}/lynx.cfg
CONF_FILES+=		share/examples/lynx/lynx.lss ${PKG_SYSCONFDIR}/lynx.lss

# But no dependency on Perl.
REPLACE_PERL+=			samples/mailto-form.pl
REPLACE_PERL+=			samples/mhtml-tool
CHECK_INTERPRETER_SKIP+=	share/lynx/lynx_doc/samples/mailto-form.pl
CHECK_INTERPRETER_SKIP+=	share/lynx/lynx_doc/samples/mhtml-tool
CHECK_WRKREF_SKIP+=		bin/lynx

.include "options.mk"

# For nls/gettext.  gettext-tools needs dlopen(), which the OpenVMS C RTL
# does not provide.
.if ${OPSYS} == "OpenVMS"
CONFIGURE_ARGS+=	--disable-nls
.else
PLIST.nls=		yes
USE_TOOLS+=		msgfmt
CONFIGURE_ARGS+=	--enable-nls
CONFIGURE_ARGS+=	--with-nls-datadir=${PREFIX}/${PKGLOCALEDIR}
CONFIGURE_ARGS+=	--without-included-gettext
.include "../../devel/gettext-tools/buildlink3.mk"
.include "../../devel/gettext-lib/buildlink3.mk"
.endif

# Note: --enable-{default-colors,scrollbar} are simply ignored
# for certain settings of --with-screen.
CONFIGURE_ARGS+=	--with-screen=${SCREENTYPE:Q}
CONFIGURE_ARGS+=	--enable-default-colors
CONFIGURE_ARGS+=	--enable-scrollbar

# Install gzip'ed help files to ${PREFIX}/share/lynx
CONFIGURE_ARGS+=	--datadir="${PREFIX}/share/lynx"
CONFIGURE_ARGS+=	--enable-gzip-help --with-zlib

# Miscellaneous
CONFIGURE_ARGS+=	--enable-externs
CONFIGURE_ARGS+=	--enable-source-cache
CONFIGURE_ARGS+=	--sysconfdir=${PKG_SYSCONFDIR}

# Experimental features
CONFIGURE_ARGS+=	--enable-file-upload
CONFIGURE_ARGS+=	--enable-justify-elts
CONFIGURE_ARGS+=	--enable-kbd-layout
CONFIGURE_ARGS+=	--enable-nested-tables
CONFIGURE_ARGS+=	--enable-prettysrc
CONFIGURE_ARGS+=	--enable-read-eta

.if !empty(USE_CROSS_COMPILE:M[yY][eE][sS])
CONFIGURE_ENV+=		BUILD_CC=${NATIVE_CC:Q}
.endif

CONFIGURE_ENV.SunOS+=	cf_cv_xopen_source=no
CONFIGURE_ENV.OpenVMS+=	ac_cv_func_readdir=no
CONFIGURE_ENV.OpenVMS+=	ac_cv_header_stdc=yes
CONFIGURE_ENV.OpenVMS+=	ac_cv_lib_dir_opendir=no
CONFIGURE_ENV.OpenVMS+=	ac_cv_type_uid_t=yes
CONFIGURE_ENV.OpenVMS+=	cf_cv_netlibs=
CONFIGURE_ENV.OpenVMS+=	cf_cv_need_xopen_extension=NCURSES_WIDECHAR
CPPFLAGS.OpenVMS+=	-DNOUSERS -DTCPIP_SERVICES -Dreadonly=lynx_readonly
MAKE_FLAGS.OpenVMS+=	BUILD_CPPFLAGS=${CPPFLAGS:Q}
MAKE_ENV.OpenVMS+=	GNV_OPT_DIR=${WRKDIR}/.vms_opt

.if ${OPSYS} == "OpenVMS"
post-extract: vms-opt-setup

.PHONY: vms-opt-setup
vms-opt-setup:
	${MKDIR} ${WRKDIR}/.vms_opt
	${CP} '${FILESDIR}/gnv$$lynx.opt' '${WRKDIR}/.vms_opt/'
.  if !empty(PKG_OPTIONS:Mopenssl)
	${CAT} ${OPENSSL_OPENVMS_GNV_OPT} >> \
	    '${WRKDIR}/.vms_opt/gnv$$lynx.opt'
.  endif

.  if !empty(PKG_OPTIONS:Mopenssl)
SUBST_CLASSES+=		vms-ca
SUBST_STAGE.vms-ca=	pre-configure
SUBST_FILES.vms-ca=	lynx.cfg
SUBST_SED.vms-ca=	-e 's|^\#SSL_CERT_FILE:/etc/ssl/certs/ca-certificates.crt|SSL_CERT_FILE:${PREFIX}/share/mozilla-rootcerts/cacert.pem|'
.  endif

SUBST_CLASSES+=		makeuctb-link
SUBST_STAGE.makeuctb-link=	post-configure
SUBST_FILES.makeuctb-link=	src/chrtrans/makefile
SUBST_SED.makeuctb-link=	-e 's|-o $$@ $$(OBJS)|-o $$@ $$(srcdir)/makeuctb.c|'

SUBST_CLASSES+=		vms-config
SUBST_STAGE.vms-config=	post-configure
SUBST_FILES.vms-config=	lynx_cfg.h
SUBST_SED.vms-config=	-e 's|^\#define UNIX 1|/* \#undef UNIX */|'

# The Unix makefile omits the VMS filesystem and built-in WAIS modules that
# Lynx's native OpenVMS build includes in its WWW library.
SUBST_CLASSES+=		vms-www
SUBST_STAGE.vms-www=	post-configure
SUBST_FILES.vms-www=	WWW/Library/Implementation/makefile
SUBST_SED.vms-www=	-e 's|$$(LOB)/HTDOS$$o$$|$$(LOB)/HTDOS$$o \
	$$(LOB)/HTVMSUtils$$o $$(LOB)/HTWAIS$$o \
	$$(LOB)/HTVMS_WaisUI$$o $$(LOB)/HTVMS_WaisProt$$o|'

# GNV tar can archive individual files, but does not descend directory
# arguments on OpenVMS.  Complete the recursive part of install-doc with cp
# and then apply the compression that the upstream target attempted.
post-install:
	${INSTALL_DATA} ${WRKSRC}/CHANGES ${WRKSRC}/COPYHEADER \
		${WRKSRC}/COPYHEADER.asc ${WRKSRC}/COPYING \
		${WRKSRC}/COPYING.asc ${WRKSRC}/PROBLEMS ${WRKSRC}/README \
		${DESTDIR}${PREFIX}/share/lynx/lynx_doc/
	${CP} -R ${WRKSRC}/docs ${WRKSRC}/samples ${WRKSRC}/test \
		${DESTDIR}${PREFIX}/share/lynx/lynx_doc/
	${TOOLS_GZIP_CMD} -f \
		${DESTDIR}${PREFIX}/share/lynx/lynx_doc/docs/CHANGES*.[0-9] \
		${DESTDIR}${PREFIX}/share/lynx/lynx_doc/docs/*.announce
.endif

.include "../../devel/zlib/buildlink3.mk"
.include "../../mk/bsd.pkg.mk"
