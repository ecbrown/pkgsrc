$NetBSD$

Use pkgsrc's bundled regex implementation on OpenVMS, whose C library does
not provide the POSIX regex.h header.

--- dba_read.c.orig	2021-09-23 18:03:23.000000000 +0000
+++ dba_read.c
@@ -24 +24,5 @@
-#include <regex.h>
+#ifdef __VMS
+#include <nbcompat/regex.h>
+#else
+#include <regex.h>
+#endif
