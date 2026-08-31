# $NetBSD: Makefile,v 1.124 2022/08/03 13:06:32 jperkin Exp $

###########################################################################
###########################################################################
#
# HEADS UP!  DO NOT CHANGE THE VERSION OR PKGREVISION WITHOUT READING THIS:
#
###########################################################################
###########################################################################
#
# This package is maintained specially in order to preserve pkgsrc
# specific OS changes that diverge from the "out of the box" libtool.
# In order to keep this package in a maintainable condition, the .m4 files
# must be manipulated in order to generate the patch-* files.
#
# See devel/libtool/patches/manual.README for instructions on how to make
# these patch files properly; otherwise your changes WILL be lost on the
# next libtool update.
#
# DO NOT MAKE CHANGES TO THESE FILES:
#
#   patches/patch-build-aux_ltmain.sh
#   patches/patch-configure
#   patches/patch-libltdl_configure
#
# WITHOUT FOLLOWING THESE INSTRUCTIONS.  There are no exceptions to this rule.
#
###########################################################################
###########################################################################

.include "../../devel/libtool/Makefile.common"

PKGNAME=	${DISTNAME:S/-/-base-/}
PKGREVISION=	2

COMMENT=	Generic shared library support script (the script itself)

CONFLICTS+=	libtool<=1.3.5nb11

TEST_TARGET=	check

OVERRIDE_DIRDEPTH.install-sh=	1

# Always build libraries and executables that use the runtime linker.
# in addition, disable libtool locking, as the test is broken on AIX,
# and results in files being locked indefinitely.
LDFLAGS.AIX+=		-Wl,-brtl
CONFIGURE_ARGS.AIX+=	--disable-libtool-lock

# The MIPSpro compiler doesn't support -c with -o, but the locking
# workaround is itself broken.  Disable it unconditionally.
CONFIGURE_ARGS.IRIX+=	--disable-libtool-lock

CFLAGS.SunOS+=	${_COMPILER_ABI_FLAG.${ABI}}

# We are going to want libtool to find the same versions of the C, C++,
# and Fortran compilers.
#
USE_LANGUAGES=		c c++
CONFIGURE_ARGS+=	--disable-ltdl-install
CONFIGURE_ARGS+=	F77=no FC=no
CONFIGURE_ARGS.OpenVMS+=	--disable-shared --enable-static

# GNV tar cannot restore timestamps on read-only archive members, so the
# OpenVMS extraction path deliberately gives extracted files fresh mtimes.
# Do not let the resulting sub-second ordering request a maintainer-only
# regeneration of the distributed aclocal.m4.
MAKE_FLAGS.OpenVMS+=	ACLOCAL=true

# GNV Bash 4.4 breaks Unix directory traversal after `set -o posix` (even
# `cd .` reports ENOTDIR).  Libtool enables that mode when Bash advertises it.
# GNV env(1) cannot launch scripts by their shebang, and Awk's system() output
# does not inherit a surrounding shell redirection.  Run recursive inlining
# through Bash and collect it through an Awk pipe instead.
SUBST_CLASSES.OpenVMS+=	gnvbash
SUBST_STAGE.gnvbash=	post-patch
SUBST_FILES.gnvbash=	build-aux/funclib.sh build-aux/ltmain.in
SUBST_FILES.gnvbash+=	build-aux/ltmain.sh
SUBST_FILES.gnvbash+=	build-aux/inline-source
SUBST_SED.gnvbash=	-e 's/set -o posix/:/g'
SUBST_SED.gnvbash+=	-e '1s|/bin/sh|/bin/bash|'
SUBST_SED.gnvbash+=	-e 's|system (sprintf ("env |system (sprintf ("bash -c \\"AWK=awk SED=/bin/sed GREP=/bin/grep |'
SUBST_SED.gnvbash+=	-e 's|%s", magic, file));|%s\\\"", magic, file));|'
SUBST_SED.gnvbash+=	-e 's@system (sprintf (\(.*\));@command = sprintf (\1; while ((command | getline line) > 0) print line; close(command);@'
SUBST_SED.gnvbash+=	-e 's|, file)); while|, file); while|g'
# Static-only OpenVMS links produce native images rather than libtool wrapper
# scripts.  GNV's pipeline predicate can misclassify such an image during
# installation and then try to source it, so make the wrapper test false.
SUBST_SED.gnvbash+=	-e 's,^      $$lt_truncate_bin < "$$1" 2>/dev/null | func_generated_by_libtool_p$$,      false,'
MAKE_ENV.OpenVMS+=	AWK=awk

USE_TOOLS+=		echo

REPLACE_SH+=	libtoolize.in

.PHONY: fix-libtool
fix-libtool:
	cd ${WRKSRC}; for f in libtool; do				\
		${SED}	-e "s,-L${BUILDLINK_DIR}/lib *,,g"		\
			-e "s,${BUILDLINK_DIR}/lib *,,g"		\
			$$f > $$f.new;					\
		if [ -x $$f ]; then ${CHMOD} +x $$f.new; fi;		\
		${MV} -f $$f.new $$f;					\
	done

post-build: fix-libtool

post-build:
	@${SED}	-e "s|@PREFIX@|"${PREFIX:Q}"|g"				\
		-e "s|@SH@|"${SH:Q}"|g"					\
		${FILESDIR}/shlibtool.in > ${WRKSRC}/shlibtool

post-install:
	${INSTALL_SCRIPT} ${WRKSRC}/shlibtool ${DESTDIR}${PREFIX}/bin/shlibtool

BUILDLINK_DEPMETHOD.dlcompat=	build

# OpenVMS has no dlopen interface.  libtool-base itself is still useful for
# static archives and is configured above not to install libltdl.
.if ${OPSYS} != "OpenVMS"
.include "../../mk/dlopen.buildlink3.mk"
.endif
.include "../../mk/bsd.pkg.mk"
