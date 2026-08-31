# $NetBSD$
#
# System-supplied tools for VSI OpenVMS using the GNV environment.

TOOLS_PLATFORM.[?=		[			# shell builtin
TOOLS_PLATFORM.awk?=		/bin/awk
TOOLS_PLATFORM.basename?=	/bin/basename
TOOLS_PLATFORM.bash?=		/bin/bash
TOOLS_PLATFORM.bison?=		/bin/bison
TOOLS_PLATFORM.bison-yacc?=	/bin/bison -y
TOOLS_PLATFORM.bzcat?=		/bin/bzip2 -dc
TOOLS_PLATFORM.bzip2?=		/bin/bzip2
TOOLS_PLATFORM.cat?=		/bin/cat
TOOLS_PLATFORM.chgrp?=		/bin/chgrp
TOOLS_PLATFORM.chmod?=		/bin/chmod
TOOLS_PLATFORM.chown?=		/bin/chown
TOOLS_PLATFORM.cmp?=		/bin/cmp
TOOLS_PLATFORM.cp?=		/bin/cp
TOOLS_PLATFORM.cut?=		/bin/cut
TOOLS_PLATFORM.date?=		/bin/date
TOOLS_PLATFORM.diff?=		/bin/diff
TOOLS_PLATFORM.dirname?=	/bin/dirname
TOOLS_PLATFORM.echo?=		echo			# shell builtin
TOOLS_PLATFORM.egrep?=		/bin/egrep
TOOLS_PLATFORM.env?=		/bin/env
TOOLS_PLATFORM.expr?=		/bin/expr
TOOLS_PLATFORM.false?=		false			# shell builtin
TOOLS_PLATFORM.fgrep?=		/bin/fgrep
TOOLS_PLATFORM.find?=		/bin/find
TOOLS_PLATFORM.flex?=		/bin/flex
TOOLS_PLATFORM.gawk?=		/bin/gawk
TOOLS_PLATFORM.git?=		/bin/git
# GNV's bundled GNU make 3.78.1 is older than pkgsrc's minimum requirement.
# Leave gmake unresolved so the tools framework selects devel/gmake.
TOOLS_PLATFORM.grep?=		/bin/grep
TOOLS_PLATFORM.gsed?=		/bin/sed
TOOLS_PLATFORM.gtar?=		/bin/tar
TOOLS_PLATFORM.head?=		/bin/head
TOOLS_PLATFORM.hostname?=	/bin/hostname
TOOLS_PLATFORM.id?=		/bin/id
TOOLS_PLATFORM.lex?=		/bin/flex
TOOLS_PLATFORM.ln?=		/bin/ln
TOOLS_PLATFORM.ls?=		/bin/ls
TOOLS_PLATFORM.mkdir?=		/bin/mkdir -p
TOOLS_PLATFORM.mktemp?=		/bin/mktemp
TOOLS_PLATFORM.mv?=		/bin/mv
TOOLS_PLATFORM.nice?=		/bin/nice
TOOLS_PLATFORM.patch?=		/bin/patch
# The VSI kit supplies a native Perl used by bootstrap tools and by packages
# that need to translate GNV paths before building a native VMS image.
TOOLS_PLATFORM.perl?=		/PERL_ROOT/000000/PERL.EXE
TOOLS_PLATFORM.printf?=		/bin/printf
TOOLS_PLATFORM.pwd?=		/bin/pwd
TOOLS_PLATFORM.readlink?=	/bin/readlink
TOOLS_PLATFORM.rm?=		/bin/rm
TOOLS_PLATFORM.rmdir?=		/bin/rmdir
TOOLS_PLATFORM.sdiff?=		/bin/sdiff
TOOLS_PLATFORM.sed?=		/bin/sed
TOOLS_PLATFORM.sh?=		/bin/bash
TOOLS_PLATFORM.sleep?=		/bin/sleep
TOOLS_PLATFORM.sort?=		/bin/sort
TOOLS_PLATFORM.tail?=		/bin/tail
TOOLS_PLATFORM.tar?=		/bin/tar
TOOLS_PLATFORM.tee?=		/bin/tee
TOOLS_PLATFORM.test?=		test			# shell builtin
TOOLS_PLATFORM.touch?=		/bin/touch
TOOLS_PLATFORM.tr?=		/bin/tr
TOOLS_PLATFORM.true?=		true			# shell builtin
TOOLS_PLATFORM.tsort?=		/bin/tsort
TOOLS_PLATFORM.uniq?=		/bin/uniq
TOOLS_PLATFORM.uname?=		/bin/uname
TOOLS_PLATFORM.unzip?=		/bin/unzip
TOOLS_PLATFORM.wc?=		/bin/wc
TOOLS_PLATFORM.xargs?=		/bin/xargs
TOOLS_PLATFORM.yacc?=		/bin/bison -y
TOOLS_PLATFORM.zip?=		/bin/zip
