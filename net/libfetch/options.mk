# $NetBSD: options.mk,v 1.8 2026/04/16 08:32:59 wiz Exp $

PKG_OPTIONS_VAR=	PKG_OPTIONS.libfetch
PKG_SUPPORTED_OPTIONS=	inet6 openssl

.include "../../mk/bsd.prefs.mk"

CHECK_BUILTIN.openssl:=	yes
.include "../../security/openssl/builtin.mk"
CHECK_BUILTIN.openssl:=	no

.if ${USE_BUILTIN.openssl:U:tl} == "yes"
PKG_SUGGESTED_OPTIONS+=	openssl
.endif

.if ${IPV6_READY:tl} == "yes"
PKG_SUGGESTED_OPTIONS+=	inet6
.endif

.include "../../mk/bsd.options.mk"

.if !empty(PKG_OPTIONS:Minet6)
MAKE_ENV+=		FETCH_WITH_INET6=yes
.else
MAKE_ENV+=		FETCH_WITH_INET6=no
.endif

.if !empty(PKG_OPTIONS:Mopenssl)
MAKE_ENV+=		FETCH_WITH_OPENSSL=yes

.include "../../security/openssl/buildlink3.mk"
.  if ${OPSYS} == "OpenVMS"
DEPENDS+=		mozilla-rootcerts>=1.1.20260715:../../security/mozilla-rootcerts
CPPFLAGS+=		-DSSL_CA_CERT_FILE="\"${PREFIX}/share/mozilla-rootcerts/cacert.pem\""
.  endif
.else
MAKE_ENV+=		FETCH_WITH_OPENSSL=no
.endif
