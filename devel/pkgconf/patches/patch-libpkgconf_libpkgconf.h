$NetBSD$

VSI C does not implement GCC's packed attribute.  This structure contains an
enum followed by a uint32_t and naturally has the bytecode format's required
layout on OpenVMS, so no implementation-specific packing directive is needed.

--- libpkgconf/libpkgconf.h.orig	2026-08-23 18:53:18.000000000 +0000
+++ libpkgconf/libpkgconf.h
@@ -120,7 +120,9 @@ struct pkgconf_bufferset_ {
 	pkgconf_buffer_t buffer;
 };
 
-#if defined(_MSC_VER) && !defined(__clang__)
+#if defined(__VMS)
+# define PKGCONF_PACKED_STRUCT(name) struct name
+#elif defined(_MSC_VER) && !defined(__clang__)
 # define PKGCONF_PACKED_STRUCT(name) __pragma(pack(push, 1)) struct name __pragma(pack(pop))
 #else
 # define PKGCONF_PACKED_STRUCT(name) struct __attribute__((__packed__)) name
