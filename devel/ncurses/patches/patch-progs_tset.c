$NetBSD$

Declare ttyname explicitly on OpenVMS.  VSI supplies the routine but does not
expose its prototype under the feature environment used by ncurses; an
implicit int return truncates the pointer on x86-64.

--- progs/tset.c.orig	2025-11-01 20:16:41.000000000 +0000
+++ progs/tset.c
@@ -94,7 +94,7 @@
 #if HAVE_GETTTYNAM
 #include <ttyent.h>
 #endif
-#ifdef NeXT
+#if defined(NeXT) || defined(__VMS)
 char *ttyname(int fd);
 #endif
 
