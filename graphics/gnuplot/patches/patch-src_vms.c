$NetBSD$

OpenVMS x86-64 uses the current VSI C RTL implementations of popen(3),
pclose(3), waitpid(2), and strftime(3).  Keep gnuplot's compatibility
implementations for older runtimes only, and let the GNV POSIX input path avoid
case-sensitive SMG and LIB$ external names.

--- src/vms.c.orig	2025-10-21 17:28:21.000000000 +0000
+++ src/vms.c
@@ -36,6 +36,7 @@
 
 #include "syscfg.h"     /* for the prototypes */
 #include "stdfn.h"
+#include "term_api.h"
 
 
 /* these are needed to modify terminal characteristics */
@@ -56,7 +57,7 @@
 int vms_vkid;	/* Virtual keyboard id */
 int vms_ktid;	/* key table id, for translating keystrokes */
 
-#ifdef PIPES
+#if defined(PIPES) && !defined(DECCRTL)
 
 /* (to aid porting) - how are errors dealt with */
 
@@ -259,7 +260,7 @@
 
 }  /* end of waitpid() */
 
-#endif /* PIPES */
+#endif /* PIPES && !DECCRTL */
 
 /*
  * Since vax/vms is the only remaining user of locally-coded strftime,
@@ -522,8 +523,10 @@
 	cur_char_buf[2] &= ~TT2$M_FALLBACK;
 	sys$qiow(0, chan, IO$_SETMODE, 0, 0, 0, cur_char_buf, 12, 0, 0, 0, 0);
     } else {
+#ifndef GP_OPENVMS_POSIX
 	if (status != SS$_NORMAL)
 	    lib$signal(status, 0, 0);
+#endif
     }
     sys$dassgn(chan);
 }
@@ -593,11 +596,14 @@
 void
 vms_init_screen()
 {
+#ifdef GP_OPENVMS_POSIX
+    return;
+#else
     /* initialise screen management routines for command recall */
     unsigned int ierror;
     if (ierror = smg$create_virtual_keyboard(&vms_vkid) != SS$_NORMAL)
 	done(ierror);
     if (ierror = smg$create_key_table(&vms_ktid) != SS$_NORMAL)
 	done(ierror);
+#endif
 }
-
