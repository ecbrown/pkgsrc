# $NetBSD$
#
# Compiler definition for VSI C on OpenVMS.  VSI_CC names GNV's native gcc
# compatibility frontend.  The pkgsrc wrapper supplies the frontend's
# object-only link workaround without adding another shell process.

.if !defined(COMPILER_VSI_MK)
COMPILER_VSI_MK=	defined

.include "../../mk/bsd.prefs.mk"

VSI_CC?=		/bin/gcc.exe

LANGUAGES.vsi=		c
CC_VERSION?=		VSI-C

CCPATH=			${VSI_CC}
PKG_CC:=		${VSI_CC}
PKG_CPP:=		${VSI_CC} -E
_COMPILER_STRIP_VARS+=	CC

_LANGUAGES.vsi=	# empty
.for _lang_ in ${USE_LANGUAGES}
_LANGUAGES.vsi+=	${LANGUAGES.vsi:M${_lang_}}
.endfor

.endif	# COMPILER_VSI_MK
