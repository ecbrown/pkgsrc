# $NetBSD$
#
# System-supplied tools for VSI OpenVMS using the GNV environment.

TOOLS_PLATFORM.[?=		[			# shell builtin
# GNV executables have an explicit .exe suffix on disk.  The C RTL supplies
# that suffix for direct command lookup, but not when a pkgsrc tool symlink is
# followed, so the platform paths must name the physical files.
TOOLS_PLATFORM.awk?=		/bin/awk.exe
TOOLS_PLATFORM.basename?=	/bin/basename.exe
TOOLS_PLATFORM.bash?=		/bin/bash.exe
TOOLS_PLATFORM.bison?=		/bin/bison.exe
TOOLS_PLATFORM.bison-yacc?=	/bin/bison.exe -y
TOOLS_PLATFORM.bzcat?=		/bin/bzip2.exe -dc
TOOLS_PLATFORM.bzip2?=		/bin/bzip2.exe
TOOLS_PLATFORM.cat?=		/bin/cat.exe
TOOLS_PLATFORM.chgrp?=		/bin/chgrp.exe
TOOLS_PLATFORM.chmod?=		/bin/chmod.exe
TOOLS_PLATFORM.chown?=		/bin/chown.exe
TOOLS_PLATFORM.cmp?=		/bin/cmp.exe
TOOLS_PLATFORM.cp?=		/bin/cp.exe
TOOLS_PLATFORM.cut?=		/bin/cut.exe
TOOLS_PLATFORM.date?=		/bin/date.exe
TOOLS_PLATFORM.diff?=		/bin/diff.exe
TOOLS_PLATFORM.dirname?=	/bin/dirname.exe
TOOLS_PLATFORM.echo?=		echo			# shell builtin
TOOLS_PLATFORM.egrep?=		/bin/egrep.exe
TOOLS_PLATFORM.env?=		/bin/env.exe
TOOLS_PLATFORM.expr?=		/bin/expr.exe
TOOLS_PLATFORM.false?=		false			# shell builtin
TOOLS_PLATFORM.fgrep?=		/bin/fgrep.exe
TOOLS_PLATFORM.find?=		/bin/find.exe
TOOLS_PLATFORM.flex?=		/bin/flex.exe
TOOLS_PLATFORM.gawk?=		/bin/gawk.exe
TOOLS_PLATFORM.git?=		/bin/git.exe
# GNV's bundled GNU make 3.78.1 is older than pkgsrc's minimum requirement.
# Leave gmake unresolved so the tools framework selects devel/gmake.
TOOLS_PLATFORM.grep?=		/bin/grep.exe
TOOLS_PLATFORM.gsed?=		/bin/sed.exe
TOOLS_PLATFORM.gtar?=		/bin/tar.exe
TOOLS_PLATFORM.head?=		/bin/head.exe
TOOLS_PLATFORM.hostname?=	/bin/hostname.exe
TOOLS_PLATFORM.id?=		/bin/id.exe
TOOLS_PLATFORM.lex?=		/bin/flex.exe
TOOLS_PLATFORM.ln?=		/bin/ln.exe
TOOLS_PLATFORM.ls?=		/bin/ls.exe
TOOLS_PLATFORM.mkdir?=		/bin/mkdir.exe -p
TOOLS_PLATFORM.mktemp?=		/bin/mktemp.exe
TOOLS_PLATFORM.mv?=		/bin/mv.exe
TOOLS_PLATFORM.nice?=		/bin/nice.exe
TOOLS_PLATFORM.openssl?=	${_PKGSRC_TOPDIR}/mk/tools/openssl.OpenVMS
TOOLS_PLATFORM.patch?=		/bin/patch.exe
# The VSI kit supplies a native Perl used by bootstrap tools and by packages
# that need to translate GNV paths before building a native VMS image.
TOOLS_PLATFORM.perl?=		/PERL_ROOT/000000/PERL.EXE
TOOLS_PLATFORM.printf?=		/bin/printf.exe
TOOLS_PLATFORM.pwd?=		/bin/pwd.exe
TOOLS_PLATFORM.readlink?=	/bin/readlink.exe
TOOLS_PLATFORM.rm?=		/bin/rm.exe
TOOLS_PLATFORM.rmdir?=		/bin/rmdir.exe
TOOLS_PLATFORM.sdiff?=		/bin/sdiff.exe
TOOLS_PLATFORM.sed?=		/bin/sed.exe
TOOLS_PLATFORM.sh?=		/bin/bash.exe
TOOLS_PLATFORM.sleep?=		/bin/sleep.exe
TOOLS_PLATFORM.sort?=		/bin/sort.exe
TOOLS_PLATFORM.tail?=		/bin/tail.exe
TOOLS_PLATFORM.tar?=		/bin/tar.exe
TOOLS_PLATFORM.tee?=		/bin/tee.exe
TOOLS_PLATFORM.test?=		test			# shell builtin
TOOLS_PLATFORM.touch?=		/bin/touch.exe
TOOLS_PLATFORM.tr?=		/bin/tr.exe
TOOLS_PLATFORM.true?=		true			# shell builtin
TOOLS_PLATFORM.tsort?=		/bin/tsort.exe
TOOLS_PLATFORM.uniq?=		/bin/uniq.exe
TOOLS_PLATFORM.uname?=		/bin/uname.exe
TOOLS_PLATFORM.unzip?=		/bin/unzip.exe
TOOLS_PLATFORM.wc?=		/bin/wc.exe
TOOLS_PLATFORM.xargs?=		/bin/xargs.exe
TOOLS_PLATFORM.yacc?=		/bin/bison.exe -y
TOOLS_PLATFORM.zip?=		/bin/zip.exe
