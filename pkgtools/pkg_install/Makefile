# $NetBSD: Makefile,v 1.245 2025/07/01 12:16:14 jperkin Exp $

# Notes to package maintainers:
#
# Updating this package does not automatically necessitate bumping
# PKGTOOLS_REQD in bsd.pkg.mk. Do so if and only if there is a critical
# change in the pkg_* tools that pkgsrc relies on for proper operation.

PKGNAME=		pkg_install-${VERSION}
PKGREVISION=		1
CATEGORIES=		pkgtools

MAINTAINER=		agc@NetBSD.org
HOMEPAGE=		https://www.pkgsrc.org/
COMMENT=		Package management and administration tools for pkgsrc
LICENSE=		modified-bsd

POLICY_UPDATE_LIMITED=	bootstrap

BOOTSTRAP_PKG=		yes
SKIP_LICENSE_CHECK=	yes

CONFLICTS+=		audit-packages-[0-9]*

GNU_CONFIGURE=		yes
CONFIGURE_ARGS+=	--sysconfdir=${PKG_SYSCONFDIR}
CONFIGURE_ARGS+=	--with-pkgdbdir=${PKG_DBDIR}

USE_FEATURES=		nbcompat

NBCOMPAT_CONFIGURE_ARGS+=	--enable-bsd-getopt --enable-db

SKIP_AUDIT_PACKAGES=	yes
NO_PKGTOOLS_REQD_CHECK=	yes
PKG_PRESERVE=		yes
CHECK_PERMS_SKIP=	*

# These are needed to solve a chicken-and-egg problem where pkgsrc uses
# newer features of pkg_install, but older NetBSD installations won't
# support them.  In this case, we explicitly use the native GCC
# compiler to avoid problems with depending on pkgsrc GCC for building
# pkg_install.
#
# We also use the newly built pkg_{add,create,delete} since upgrading
# from an older pkg_install might required features of the new
# program, provided that we are not cross-compiling.  (XXX If we are
# cross-compiling, we should maybe have a host build dependency on
# ourselves...)
#
# Note that the definitions are only overriden for the phases that are
# supposed to use them.  pkg_admin pmatch might be used when looking for
# dependencies. If that is ever changed in a incompatible way, this has to
# be rethought.
USE_NATIVE_GCC=		yes

.include "../../mk/bsd.prefs.mk"

.if ${OPSYS} == "OpenVMS"
# GNV's GCC does not predefine the native C RTL marker used by pkg_install's
# OpenVMS portability code.  Keep the VMS path and file-version handling
# enabled when building the tools on VSI OpenVMS.
CPPFLAGS+=	-D__VMS
# A value returned from main is otherwise interpreted as a native VMS
# condition code, where EXIT_FAILURE (1) is an odd success status.
CFLAGS+=	-Wc/MAIN=POSIX_EXIT
.endif

.if ${OS_VARIANT} == "SCOOSR5"
# SCO OpenServer 5.0.7/3.2's strings.h defines strcasecmp with
# "unsigned size_t". We cannot use -Werror for net/libfetch.
MAKE_FLAGS+=	NOGCCERROR=yes
.endif

.if defined(_PKGSRC_BARRIER) && empty(USE_CROSS_COMPILE:M[yY][eE][sS])
PKG_ADD_CMD=		${WRKSRC}/add/pkg_add
PKG_CREATE_CMD=		${WRKSRC}/create/pkg_create
PKG_DELETE_CMD=		${WRKSRC}/delete/pkg_delete
#
#  Special case for dependency checking in 'non-bootstrap' builds:
#  don't redefine 'pkg_info' if it already exists.
#
.  if defined(PKG_INFO_CMD) && !exists(${PKG_INFO_CMD})
PKG_INFO_CMD=		${WRKSRC}/info/pkg_info
.  endif
.endif

CPPFLAGS+=		-D_LARGEFILE_SOURCE -D_LARGE_FILES
CPPFLAGS+=		-D_FILE_OFFSET_BITS=64

CPPFLAGS+=		-DDEF_UMASK=${DEF_UMASK}

MAKE_ENV+=		PKGSRC_MACHINE_ARCH=${MACHINE_ARCH}
MAKE_ENV+=		OPSYS=${OPSYS}
MAKE_ENV+=		CATMAN_SECTION_SUFFIX=${CATMAN_SECTION_SUFFIX:Q}
MAKE_ENV+=		MANINSTALL=${MANINSTALL:Q}
MAKE_ENV+=		SED=${SED:Q}

PKG_DBDIR?=		${PREFIX}/pkgdb
EGDIR=			${PREFIX}/share/examples/pkg_install

PLIST_SUBST+=		PKG_DBDIR=${PKG_DBDIR}

DEINSTALL_SRC=		${PKGDIR}/DEINSTALL
INSTALL_SRC=		${PKGDIR}/INSTALL
FILES_SUBST+=		PKG_DBDIR=${PKG_DBDIR}			\
			PKG_TOOLS_BIN=${PKG_TOOLS_BIN:Q}	\
			MKDIR=${MKDIR:Q}

.include "../../mk/compiler.mk"

.if !empty(CC_VERSION:Mclang*)
MAKE_FLAGS+=		ACTIVE_CC=clang
.endif

VERSION!=		${AWK} '/PKGTOOLS_VERSION/ {print $$3}' \
			${FILESDIR}/lib/version.h

# Requires 3.x filter API and bugfixes.
BUILDLINK_API_DEPENDS.libarchive+=	libarchive>=3.3.1

.include "../../archivers/bzip2/builtin.mk"
.include "../../archivers/xz/builtin.mk"
.include "../../archivers/libarchive/builtin.mk"
.include "../../devel/zlib/builtin.mk"
.include "../../security/openssl/builtin.mk"

.include "options.mk"

.if ${OPSYS} == "OpenVMS" && !empty(PKG_OPTIONS:Mopenssl)
MAKE_ENV+=		GNV_OPT_DIR=${WRKDIR}/.vms_opt

post-extract: openvms-openssl-options

.PHONY: openvms-openssl-options
openvms-openssl-options:
	${MKDIR} ${WRKDIR}/.vms_opt
.  for prog in pkg_add pkg_admin pkg_create pkg_delete pkg_info
	${CP} ${OPENSSL_OPENVMS_GNV_OPT} \
	    '${WRKDIR}/.vms_opt/gnv$$${prog}.opt'
.  endfor
.endif

.if empty(USE_BUILTIN.bzip2:M[yY][eE][sS]) || \
    empty(USE_BUILTIN.zlib:M[yY][eE][sS])
USE_BUILTIN.libarchive=	no
.endif

FILESDIR.bzip2?=	${.CURDIR}/../../archivers/bzip2/files
FILESDIR.libarchive?=	${.CURDIR}/../../archivers/libarchive/files
FILESDIR.zlib?=		${.CURDIR}/../../devel/zlib/files
FILESDIR.libfetch?=	${.CURDIR}/../../net/libfetch/files
FILESDIR.netpgpverify?=	${.CURDIR}/../../security/netpgpverify/files

.if empty(USE_BUILTIN.bzip2:M[yY][eE][sS])
CPPFLAGS+=	-I${WRKDIR}/bzip2
LDFLAGS+=	-L${WRKDIR}/bzip2
.endif
.if empty(USE_BUILTIN.zlib:M[yY][eE][sS])
CPPFLAGS+=	-I${WRKDIR}/zlib
LDFLAGS+=	-L${WRKDIR}/zlib
.endif
.if empty(USE_BUILTIN.libarchive:M[yY][eE][sS])
CPPFLAGS+=	-I${WRKDIR}/libarchive/libarchive
LDFLAGS+=	-L${WRKDIR}/libarchive/.libs

.  if ${OPSYS} == "OpenVMS"
# VSI's linker accepts missing -l arguments, which makes the PCRE probes
# produce false positives and leak unusable libraries into libarchive.pc.
LIBARCHIVE_CONFIGURE_ARGS+=	--disable-posix-regex-lib
# The unconditional libmd probe has the same false-positive behavior.  Its
# follow-up header checks prevent a link leak today, but HAVE_LIBMD is wrong.
LIBARCHIVE_CONFIGURE_ENV+=	ac_cv_lib_md_MD5Init=no
# The linked compatibility library exports timegm, but VSI time.h does not
# declare it.  Use libarchive's fallback instead of an implicit-int call that
# can truncate time_t.
LIBARCHIVE_CONFIGURE_ENV+=	ac_cv_func_timegm=no
# VSI's linker also returns success for unresolved function references.  Cache
# APIs that are either undeclared or unavailable so libarchive selects its
# portability paths and does not leave unresolved symbols in the final images.
LIBARCHIVE_CONFIGURE_ENV+=	ac_cv_func_closefrom=no
LIBARCHIVE_CONFIGURE_ENV+=	ac_cv_func_fdopendir=no
LIBARCHIVE_CONFIGURE_ENV+=	ac_cv_func_fchdir=no
LIBARCHIVE_CONFIGURE_ENV+=	ac_cv_func_fstatat=no
LIBARCHIVE_CONFIGURE_ENV+=	ac_cv_func_futimens=no
LIBARCHIVE_CONFIGURE_ENV+=	ac_cv_func_futimes=no
LIBARCHIVE_CONFIGURE_ENV+=	ac_cv_func_futimesat=no
LIBARCHIVE_CONFIGURE_ENV+=	ac_cv_func_issetugid=no
LIBARCHIVE_CONFIGURE_ENV+=	ac_cv_func_linkat=no
LIBARCHIVE_CONFIGURE_ENV+=	ac_cv_func_lutimes=no
LIBARCHIVE_CONFIGURE_ENV+=	ac_cv_func_mkfifo=no
LIBARCHIVE_CONFIGURE_ENV+=	ac_cv_func_mknod=no
LIBARCHIVE_CONFIGURE_ENV+=	ac_cv_func_openat=no
LIBARCHIVE_CONFIGURE_ENV+=	ac_cv_func_posix_spawn=no
LIBARCHIVE_CONFIGURE_ENV+=	ac_cv_func_posix_spawnp=no
LIBARCHIVE_CONFIGURE_ENV+=	ac_cv_func_readpassphrase=no
# Missing -l arguments are only warnings to VSI's gcc driver, so disable
# optional libraries that are not installed instead of recording them in
# libarchive.pc and every pkg_install link command.
LIBARCHIVE_CONFIGURE_ENV+=	ac_cv_lib_acl_acl_get_file=no
LIBARCHIVE_CONFIGURE_ENV+=	ac_cv_lib_b2_blake2sp_init=no
LIBARCHIVE_CONFIGURE_ENV+=	ac_cv_lib_richacl_richacl_get_file=no
# GNV does not provide ranlib; VSI's librarian needs no indexing pass.
LIBARCHIVE_CONFIGURE_ENV+=	RANLIB=${TOOLS_PLATFORM.ranlib:Q}
.  endif

# Only use XZ if we actually have builtin support for it. Some systems
# install the header, but without usable library or prefer to not use
# the system version, so give them some control.
.  if empty(USE_BUILTIN.xz:M[Yy][Ee][Ss])
LIBARCHIVE_CONFIGURE_ARGS+=	--without-lzma
.  endif

LIBARCHIVE_LIBS=	${SED} -n -e 's/^Libs.private://p' \
			${WRKDIR}/libarchive/build/pkgconfig/libarchive.pc 2>/dev/null || echo
LIBS+=			${LIBARCHIVE_LIBS:sh}

CONFIG_GUESS_OVERRIDE+=	${WRKDIR}/libarchive/build/autoconf/config.guess \
			${WRKSRC}/config.guess
CONFIG_SUB_OVERRIDE+=	${WRKDIR}/libarchive/build/autoconf/config.sub \
			${WRKSRC}/config.sub

CPPFLAGS.Cygwin+=	-DLIBARCHIVE_STATIC

# Make sure that the linker used our static library instead of the
# (outdated) dynamic library "/usr/lib/libarchive.dylib".
LDFLAGS.Darwin+=	-Wl,-search_paths_first

# workaround for pkg/45491
CONFIGURE_ENV.SunOS+=	ac_cv_header_ext2fs_ext2_fs_h=no
CONFIGURE_ENV.OpenVMS+=	RANLIB=${TOOLS_PLATFORM.ranlib:Q}
# The generated configure defines its link-test helper only after this probe;
# OpenVMS has no native libdb, while libnbcompat supplies dbopen directly.
CONFIGURE_ENV.OpenVMS+=	ac_cv_lib_db___db185_open=no

# Hack to make sure that the libarchive version is replaced
pre-configure: config-guess-override config-sub-override
.endif
CPPFLAGS+=	-I${WRKDIR}/libfetch
LDFLAGS+=	-L${WRKDIR}/libfetch
# Avoid duplicate and conflicting headers, pull in any we need
# directly with <netpgpg/*.h>
CPPFLAGS+=	-I${WRKDIR}
LDFLAGS+=	-L${WRKDIR}/netpgp

CONFIGURE_ENV+=	LIBS=${LIBS:Q}

# XXX: libnbcompat provides this function, so libarchive configure thinks it
# is available, but does not use nbcompat headers and thus falls foul of GCC
# 14 implicit declaration checks later on.
#
# TODO: provide a sane environment for each inplace build, this is just
# papering over cracks, libarchive should not be using libnbcompat at all.
CONFIGURE_ENV.SunOS+=	ac_cv_func_lchmod=no

# Some platforms pre-create WRKSRC, so copy the directory contents rather
# than nesting FILESDIR below it.
do-extract:
	@${CP} -R ${FILESDIR}/. ${WRKSRC}
.if empty(USE_BUILTIN.bzip2:M[yY][eE][sS])
	@${CP} -R ${FILESDIR.bzip2} ${WRKDIR}/bzip2
.endif
.if empty(USE_BUILTIN.zlib:M[yY][eE][sS])
	@${CP} -R ${FILESDIR.zlib} ${WRKDIR}/zlib
.endif
.if empty(USE_BUILTIN.libarchive:M[yY][eE][sS])
	@${CP} -R ${FILESDIR.libarchive} ${WRKDIR}/libarchive
.endif
	@${CP} -R ${FILESDIR.libfetch} ${WRKDIR}/libfetch
	@${CP} -R ${FILESDIR.netpgpverify} ${WRKDIR}/netpgp

pre-configure:
.if empty(USE_BUILTIN.bzip2:M[yY][eE][sS])
	cd ${WRKDIR}/bzip2 && ${BUILD_MAKE_CMD} libbz2.a
.endif
.if empty(USE_BUILTIN.zlib:M[yY][eE][sS])
	cd ${WRKDIR}/zlib && ${BUILD_MAKE_CMD} libz.a
.endif
.if empty(USE_BUILTIN.libarchive:M[yY][eE][sS])
	cd ${WRKDIR}/libarchive && ${SETENV} ${_CONFIGURE_SCRIPT_ENV}	\
		${LIBARCHIVE_CONFIGURE_ENV}				\
		${CONFIG_SHELL} ${CONFIG_SHELL_FLAGS} ./configure	\
		--disable-bsdcat --disable-bsdcpio --disable-bsdtar	\
		--disable-bsdunzip --disable-shared			\
		--disable-dependency-tracking --disable-maintainer-mode	\
		--without-expat --without-xml2 --without-lz4 --without-lzo2 \
		--without-iconv --without-nettle --without-zstd --without-cng \
		${LIBARCHIVE_CONFIGURE_ARGS}
.  if ${OPSYS} == "OpenVMS"
	# Avoid GNV's hanging empty grep/sort pipelines for disabled tests.
	${TOUCH} ${WRKDIR}/libarchive/libarchive/test/list.h \
		${WRKDIR}/libarchive/tar/test/list.h \
		${WRKDIR}/libarchive/cpio/test/list.h \
		${WRKDIR}/libarchive/cat/test/list.h \
		${WRKDIR}/libarchive/unzip/test/list.h
.  endif
	cd ${WRKDIR}/libarchive && ${BUILD_MAKE_CMD}
.endif
	cd ${WRKDIR}/libfetch && ${TOUCH} fetch.cat3 &&			\
		${SETENV} ${MAKE_ENV} ${BSD_MAKE_ENV}			\
		${MAKE_PROGRAM} ${MAKE_FLAGS} ${BUILD_MAKE_FLAGS}	\
			-f ${MAKE_FILE} depend all
	cd ${WRKDIR}/netpgp && ${CP} Makefile.lib.in Makefile.in &&	\
	${CONFIG_SHELL} ${CONFIG_SHELL_FLAGS} ./configure &&		\
	${SETENV} ${MAKE_ENV} ${BSD_MAKE_ENV} ${MAKE_PROGRAM}		\
		${MAKE_FLAGS} ${BUILD_MAKE_FLAGS} -f ${MAKE_FILE} all

# XXX Reverse the order that update does things since
# XXX we need pkg_delete built before we can deinstall.
# XXX This should probably be the default order for all packages.
update:
	${MAKE}
	${MAKE} deinstall _UPDATE_RUNNING=YES
	${MAKE} ${UPDATE_TARGET}
	${MAKE} clean

update-catpages:
	for f in lib/pkgsrc.7 add/pkg_add.1 admin/pkg_admin.1 \
	    create/pkg_create.1 delete/pkg_delete.1 info/pkg_info.1 \
	    lib/pkg_summary.5 lib/pkgsrc.7; do \
	    mandoc -I os=pkgsrc -Tascii ${FILESDIR}/$$f > \
		    ${FILESDIR}/$${f%%.[157]}.cat; \
	done
	mandoc -I os=pkgsrc -Tascii ${FILESDIR}/lib/pkg_install.conf.5.in > \
	    ${FILESDIR}/lib/pkg_install.conf.cat.in

.include "../../mk/bsd.pkg.mk"
