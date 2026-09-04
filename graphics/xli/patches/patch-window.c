$NetBSD: patch-window.c,v 1.1 2025/10/12 19:34:38 mrg Exp $

Include <sys/select.h> on NetBSD for the select() prototype, and also give
an atom a type (Atom), for GCC 14.

OpenVMS declares select() and fd_set in <socket.h>.  Treat the normal
XPutImage path as not using shared memory when MIT-SHM is disabled.


--- window.c.orig	2005-02-27 16:42:39.000000000 -0800
+++ window.c	2025-10-12 12:20:48.231111513 -0700
@@ -14,6 +14,9 @@
 #include <signal.h>
 #include <errno.h>
 #include <sys/types.h>
+#if defined(VMS)
+#include <socket.h>
+#endif
 #if defined(linux)
 #include <sys/time.h>
 #endif
@@ -22,10 +25,16 @@
 #include <stropts.h>
 #include <poll.h>
 #endif
-#ifdef _AIX
+#if defined(_AIX) || defined(__NetBSD__)
 #include <sys/select.h>
 #endif
 
+#ifdef NO_XSHM
+#define XLI_NO_SHM_IMAGE(xii) 1
+#else
+#define XLI_NO_SHM_IMAGE(xii) ((xii)->shm.shmid < 0)
+#endif
+
 static Window ImageWindow = 0;
 static Window ViewportWin = 0;
 static Colormap ImageColormap;
@@ -209,7 +218,7 @@ static void cleanUpImage(Display *disp, 
 static void setViewportColormap(Display *disp, int scrn, Visual *visual)
 {
 	XSetWindowAttributes swa;
-	static cmap_atom = None;
+	static Atom cmap_atom = None;
 	Window cmap_windows[2];
 
 	if (cmap_atom == None)
@@ -630,8 +639,8 @@ char imageInWindow(DisplayInfo *dinfo, Im
 	 */
 
 	xii->drawable = ImageWindow;
-	if ((DoesBackingStore(ScreenOfDisplay(disp, scrn)) == NotUseful &&
-	     xii->shm.shmid < 0) || globals.use_pixmap) {
+	if ((DoesBackingStore(ScreenOfDisplay(disp, scrn)) == NotUseful &&
+	     XLI_NO_SHM_IMAGE(xii)) || globals.use_pixmap) {
 		if (((pixmap = ximageToPixmap(disp, ImageWindow, xii)) ==
 		     None) && globals.verbose)
 			printf("  Cannot create image in server, repaints will be ugly!\n");
@@ -651,7 +660,7 @@ char imageInWindow(DisplayInfo *dinfo, Im
 		wa_mask_img |= CWBackPixel;
 		swa_img.event_mask |= ExposureMask;
 		wa_mask_img |= CWEventMask;
-		if (xii->shm.shmid < 0) {
+		if (XLI_NO_SHM_IMAGE(xii)) {
 			swa_img.backing_store = WhenMapped;
 			wa_mask_img |= CWBackingStore;
 		}
