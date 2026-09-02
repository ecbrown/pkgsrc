$NetBSD$

Use the declared POSIX write prototype in the standalone lib-src build.

--- src/regex.c.orig	2008-01-20 01:49:24.000000000 +0000
+++ src/regex.c
@@ -176,3 +176,7 @@
 # undef REL_ALLOC
-
+
+# ifdef HAVE_UNISTD_H
+#  include <unistd.h>
+# endif
+
 # if defined STDC_HEADERS || defined _LIBC
