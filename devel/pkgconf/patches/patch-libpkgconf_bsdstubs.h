$NetBSD$

VSI C provides strdup() in the C RTL without exposing its prototype in the
default header namespace, and lacks strnlen().  Supply the declaration and a
small bounded helper used by pkgconf.

--- libpkgconf/bsdstubs.h.orig	2026-08-23 18:53:18.000000000 +0000
+++ libpkgconf/bsdstubs.h
@@ -19,6 +19,7 @@
 #define LIBPKGCONF_BSDSTUBS_H
 
 #include <stddef.h>
+#include <string.h>
 
 #include <libpkgconf/libpkgconf-api.h>
 
@@ -26,6 +27,20 @@ extern "C" {
 extern "C" {
 #endif
 
+#if defined(__VMS)
+extern char *strdup(const char *src);
+
+static inline size_t
+pkgconf_strnlen(const char *src, size_t maxlen)
+{
+	const char *end = memchr(src, '\0', maxlen);
+
+	return end != NULL ? (size_t)(end - src) : maxlen;
+}
+
+# define strnlen pkgconf_strnlen
+#endif
+
 PKGCONF_API extern size_t pkgconf_strlcpy(char *dst, const char *src, size_t siz);
 PKGCONF_API extern size_t pkgconf_strlcat(char *dst, const char *src, size_t siz);
 PKGCONF_API extern char *pkgconf_strndup(const char *src, size_t len);
