$NetBSD$

MIT-SHM is not usable for a remote DECwindows display, and the VSI
DECwindows XShm headers conflict with the C RTL's System V IPC headers.
Allow platforms to build the normal XPutImage path without XShm.

--- ddxli.h.orig	2005-02-28 01:42:39.000000000 +0100
+++ ddxli.h
@@ -10,9 +10,15 @@
 #include <X11/Xutil.h>
 #include <X11/Xatom.h>
 #include <X11/cursorfont.h>
+#ifndef NO_XSHM
 #include <sys/ipc.h>
 #include <sys/shm.h>
 #include <X11/extensions/XShm.h>
+#else
+typedef struct {
+	int unused;
+} XShmSegmentInfo;
+#endif
 
 #if defined(SYSV) || defined(VMS)
 #include <string.h>
