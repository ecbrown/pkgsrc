# $NetBSD$
#
# Variable definitions for VSI OpenVMS using the GNV environment.

# DCL commonly exports ECHO as "WRITE SYS$OUTPUT".  Do not let that symbol
# become a make variable in shell recipes.
ECHO=			echo
ECHO_N?=		${ECHO} -n
PKGLOCALEDIR?=		share
TYPE?=			type

ROOT_USER?=		SYSTEM
ROOT_GROUP?=		SYSTEM
PKG_TOOLS_BIN?=		${LOCALBASE}/sbin
DEF_UMASK?=		0022
NOLOGIN?=		/bin/false

ULIMIT_CMD_datasize?=	:
ULIMIT_CMD_stacksize?=	:
ULIMIT_CMD_memorysize?=	:

# DECwindows supplies the native X11 implementation.
X11_TYPE?=		native

# VSI OpenSSL exposes its flattened header directory through the OPENSSL
# logical.  Record the physical headers here because bmake's exists() cannot
# resolve POSIX paths containing a dollar sign such as /sys$common/ssl3.
.if exists(/OPENSSL/opensslconf.h) && exists(/OPENSSL/opensslv.h)
H_OPENSSLCONF=		/OPENSSL/opensslconf.h
H_OPENSSLV=		/OPENSSL/opensslv.h
.endif
OPENSSL_OPENVMS_GNV_OPT?=	${PKGSRCDIR}/security/openssl/files/openvms-p32-shared.opt

# GNV's gcc frontend uses case-sensitive external names, but the x86-64 kit
# does not ship its optional supplementary C RTL library.  Prefix C library
# calls so they resolve directly to the routines exported by DECC$SHR.
CFLAGS.OpenVMS+=	-Wc/PREFIX=ALL_ENTRIES

# GNV's ln cannot currently create hard links.  pkgsrc wrappers must use
# symbolic links, while the underlying ODS-5 volume still has hard links
# enabled for programs that use the C RTL directly.
WRAPPER_USE_SYMLINK=	# defined
TOOLS_ARGS.ln=		-s
COMPILER_USE_SYMLINKS=	no

_OPSYS_SYSTEM_RPATH?=	# empty
_OPSYS_LIB_DIRS?=	# supplied through OpenVMS logical names
_OPSYS_INCLUDE_DIRS?=	# supplied by the VSI C header libraries

# OpenVMS has no Unix runtime search path.  Buildlink still uses the public
# rpath variables when adding LOCALBASE library directories, so make those
# ordinary compiler/linker -L options instead of producing a bare pathname.
_OPSYS_COMPILER_RPATH_FLAG=	-L
_OPSYS_LINKER_RPATH_FLAG=	-L

_OPSYS_HAS_INET6=	yes
_OPSYS_HAS_JAVA=	no
_OPSYS_HAS_MANZ=	no
_OPSYS_PTHREAD_AUTO=	yes
# pthread.h is a module in DECC$RTLDEF.TLB rather than a pathname that
# bsd.builtin.mk can discover.  POSIX threads are provided by the C RTL and
# need no additional linker option.
IS_BUILTIN.pthread=	yes
# iconv.h is also a C RTL text-library module.  The conversion routines are in
# DECC$SHR and require no separate libiconv archive.
IS_BUILTIN.iconv=	yes
BUILTIN_PKG.iconv=	libiconv-1.19
BUILTIN_LIB_FOUND.iconv=	no
ICONV_TYPE=		native
_OPSYS_SHLIB_TYPE=	none
_PATCH_CAN_BACKUP=	no
_USE_RPATH=		no

_STRIPFLAG_CC?=		# empty
_STRIPFLAG_INSTALL?=	# empty
_OPSYS_CAN_CHECK_SHLIBS=	no
_OPSYS_SUPPORTS_CWRAPPERS=	no

# bmake's job mode uses a pipe protocol that is not reliable under GNV, even
# for "-j1".  Keep package builds in the non-job execution path.
MAKE_JOBS_SAFE=		no
MAKE_ENV+=		INSTALL_SH_CHMOD_AFTER_MOVE=yes

# The check-files pre/post targets use a multi-process find/sed/grep/sort
# pipeline.  GNV can finish producing the list yet leave the final shell
# waiting indefinitely for pipe EOF, so this check is not currently usable.
CHECK_FILES_SUPPORTED=	no
CHECK_SHLIBS_SUPPORTED=	no

# ODS-5 can make a just-created archive directory visible too late for the
# first following member, and GNV tar fails while restoring timestamps on
# read-only members.  Pre-create the conventional source directory, skip tar's
# timestamp restoration, and make the result patchable.
EXTRACT_OPTS_TAR+=	--touch
.PHONY: openvms-precreate-wrksrc openvms-writable-wrksrc
pre-extract: openvms-precreate-wrksrc
post-extract: openvms-writable-wrksrc
openvms-precreate-wrksrc:
	${RUN} ${MKDIR} ${WRKSRC}
openvms-writable-wrksrc:
	${RUN} ${CHMOD} -R u+w ${WRKSRC}

# GNV mkdir treats a final ".dir" as the native OpenVMS directory-file type:
# mkdir conftest.dir creates conftest.DIR;1, whose Unix directory name is
# conftest.  Autoconf then cannot enter conftest.dir and may cd out of WRKSRC.
_SCRIPT.configure-scripts-osdep=					\
	${SED} -e 's/conftest\.dir/conftest_dir/g' $$file > $$file.openvms; \
	${CHMOD} +x $$file.openvms;					\
	${TOUCH} -r $$file $$file.openvms;				\
	${MV} -f $$file.openvms $$file

# GNV 3.0-2F does not provide ranlib, and its strip utility is documented as
# unsafe.  VSI's librarian writes usable object libraries without ranlib.
TOOLS_PLATFORM.ranlib?=	true
TOOLS_PLATFORM.strip?=	true

# GNV Bash cannot recover its current directory after Autoconf re-executes
# itself in POSIX mode.  Source the OpenVMS guards in every package configure
# shell and skip that re-exec path.  The same helper makes Autoconf's file
# permission tests match OpenVMS executable semantics.
CONFIGURE_ENV+=		BASH_ENV=${PKGSRCDIR}/bootstrap/openvms-bash-env
CONFIGURE_ENV+=		OPENVMS_BASH_ENV=${PKGSRCDIR}/bootstrap/openvms-bash-env
CONFIGURE_ENV+=		_as_can_reexec=no

# The C RTL's default record-oriented pipe can delay EOF delivery after a
# command substitution for minutes.  DECC$STREAM_PIPE must be a job logical
# before Bash starts; a CONFIGURE_ENV entry becomes only a Unix environment
# string and does not enable the C RTL feature.  Fail before configure instead
# of leaving config.status apparently hung.
.PHONY: openvms-check-stream-pipe
pre-configure: openvms-check-stream-pipe
openvms-check-stream-pipe:
	${RUN} /bin/dcl 'EXIT 1 + (F$$TRNLNM("DECC$$STREAM_PIPE","LNM$$JOB") .NES. "ENABLE")' || \
		${FAIL_MSG} 'Define DECC$$STREAM_PIPE as ENABLE in the OpenVMS job logical table before starting Bash.'

# Autoconf's AC_FUNC_FORK probe hides the OpenVMS vfork macro and otherwise
# records a false negative.
CONFIGURE_ENV+=		ac_cv_func_vfork=yes ac_cv_func_vfork_works=yes

# Some VSI C interfaces are implemented as header macros rather than external
# symbols, so AC_CHECK_FUNCS cannot link its usual unexpanded test reference.
CONFIGURE_ENV+=		ac_cv_func_mempcpy=yes

# VSI C has no #include_next directive, and its preprocessor output does not
# give gnulib a pathname from which to recover the system header.  Use the
# OpenVMS text-module form (for example, "#include stddef") to bypass a
# gnulib wrapper and load the corresponding module from DECC$RTLDEF.TLB.
CONFIGURE_ENV+=		gl_cv_next_alloca_h=alloca
CONFIGURE_ENV+=		gl_cv_next_arpa_inet_h=inet
CONFIGURE_ENV+=		gl_cv_next_assert_h=assert
CONFIGURE_ENV+=		gl_cv_next_ctype_h=ctype
CONFIGURE_ENV+=		gl_cv_next_dirent_h=dirent
CONFIGURE_ENV+=		gl_cv_next_errno_h=errno
CONFIGURE_ENV+=		gl_cv_next_fcntl_h=fcntl
CONFIGURE_ENV+=		gl_cv_next_float_h=float
CONFIGURE_ENV+=		gl_cv_next_iconv_h=iconv
CONFIGURE_ENV+=		gl_cv_next_inttypes_h=inttypes
CONFIGURE_ENV+=		gl_cv_next_langinfo_h=langinfo
CONFIGURE_ENV+=		gl_cv_next_limits_h=limits
CONFIGURE_ENV+=		gl_cv_next_locale_h=locale
CONFIGURE_ENV+=		gl_cv_next_malloc_h=malloc
CONFIGURE_ENV+=		gl_cv_next_math_h=math
CONFIGURE_ENV+=		gl_cv_next_netdb_h=netdb
CONFIGURE_ENV+=		gl_cv_next_netinet_in_h=in
CONFIGURE_ENV+=		gl_cv_next_pthread_h=pthread
CONFIGURE_ENV+=		gl_cv_next_sched_h=sched
CONFIGURE_ENV+=		gl_cv_next_semaphore_h=semaphore
CONFIGURE_ENV+=		gl_cv_next_signal_h=signal
CONFIGURE_ENV+=		gl_cv_next_stddef_h=stddef
CONFIGURE_ENV+=		gl_cv_next_stdint_h=stdint
CONFIGURE_ENV+=		gl_cv_next_stdio_h=stdio
CONFIGURE_ENV+=		gl_cv_next_stdlib_h=stdlib
CONFIGURE_ENV+=		gl_cv_next_string_h=string
CONFIGURE_ENV+=		gl_cv_next_strings_h=strings
CONFIGURE_ENV+=		gl_cv_next_sys_ioctl_h=ioctl
CONFIGURE_ENV+=		gl_cv_next_sys_mman_h=mman
CONFIGURE_ENV+=		gl_cv_next_sys_param_h=param
CONFIGURE_ENV+=		gl_cv_next_sys_socket_h=socket
CONFIGURE_ENV+=		gl_cv_next_sys_stat_h=stat
CONFIGURE_ENV+=		gl_cv_next_sys_time_h=time
CONFIGURE_ENV+=		gl_cv_next_sys_types_h=types
CONFIGURE_ENV+=		gl_cv_next_sys_uio_h=uio
CONFIGURE_ENV+=		gl_cv_next_sys_un_h=un
CONFIGURE_ENV+=		gl_cv_next_sys_wait_h=wait
CONFIGURE_ENV+=		gl_cv_next_time_h=time
CONFIGURE_ENV+=		gl_cv_next_unistd_h=unistd
CONFIGURE_ENV+=		gl_cv_next_utime_h=utime
CONFIGURE_ENV+=		gl_cv_next_wchar_h=wchar
CONFIGURE_ENV+=		gl_cv_next_wctype_h=wctype

# Avoid an expensive or unreliable command-line length probe through DCL.
_OPSYS_MAX_CMDLEN_CMD=	${ECHO} 4096
