$NetBSD$

OpenVMS has stat(), but its C headers do not expose the POSIX S_ISLNK macro.
Skip the optional inode/symlink cache there; pkgconf still performs its normal
textual path de-duplication.

--- libpkgconf/path.c.orig	2026-08-23 18:53:18.000000000 +0000
+++ libpkgconf/path.c
@@ -20,7 +20,7 @@
 #include <libpkgconf/libpkgconf.h>
 #include <libpkgconf/path.h>
 
-#if defined(HAVE_SYS_STAT_H) && ! defined(_WIN32)
+#if defined(HAVE_SYS_STAT_H) && !defined(_WIN32) && !defined(__VMS)
 # include <sys/stat.h>
 # define PKGCONF_CACHE_INODES
 #endif
