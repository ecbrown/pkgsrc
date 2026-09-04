$NetBSD$

OpenVMS 9 provides ftruncate() in the C runtime.  Use its declaration instead
of the historical System V F_FREESP emulation, which is unavailable there.

--- xlito.c.orig	2005-02-28 01:42:39.000000000 +0100
+++ xlito.c
@@ -5,16 +5,14 @@
 #include <errno.h>
 #include "ddxli.h"
 #include "xlito.h"
-#ifndef VMS
 #include <unistd.h>
-#endif
 
 #define VERSION "1"
 #define PATCHLEVEL "02"
 
 #define PADSIZE 20		/* allow a little over-read on image load */
 
-#if (defined(SYSV) || defined (VMS)) && !defined(SVR4) && !defined(__hpux) && !defined(_CRAY)
+#if defined(SYSV) && !defined(VMS) && !defined(SVR4) && !defined(__hpux) && !defined(_CRAY)
 #define NEED_FTRUNCATE		/* define this if your system doesn't have ftruncate() */
 #endif
 
