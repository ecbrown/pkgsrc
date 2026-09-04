$NetBSD$

Use the current VSI C RTL declarations when building against DECCRTL.  The
fallback declarations belong to the older VAX C runtime.

--- src/vms.h.orig	2025-10-21 17:28:21.000000000 +0000
+++ src/vms.h
@@ -53,8 +53,8 @@
 void done(int);
 void vms_init_screen();
 
-#ifdef PIPES
-  FILE *popen(char *, char *);
+#if defined(PIPES) && !defined(DECCRTL)
+  FILE *popen(const char *, const char *);
   int pclose(FILE *);
 #endif
 
