$NetBSD: patch-send.c,v 1.1 2024/04/17 14:11:05 nat Exp $

Fix overflow caused by typo at bit depths < 8.

Allow building without MIT-SHM.  This is needed for OpenVMS DECwindows,
where the XShm and C RTL System V IPC definitions conflict; it also avoids
probing an extension that cannot be used over the remote X connection.

--- send.c.orig	2024-04-17 02:27:43.661453625 +0000
+++ send.c
@@ -91,6 +91,7 @@ static unsigned int bitsPerPixelAtDepth(
 }
 
 
+#ifndef NO_XSHM
 static int shmErrorTrap(Display *d, XErrorEvent *ev)
 {
 	GotError = 1;
@@ -99,11 +100,13 @@ static int shmErrorTrap(Display *d, XErr
 
 	return 0;
 }
+#endif
 
 
 static void createImage(XImageInfo *xii, Image *image, Visual *visual,
 	int depth, int format)
 {
+#ifndef NO_XSHM
 	if (XShmQueryExtension(xii->disp)) {
 		xii->ximage = XShmCreateImage(xii->disp, visual, depth, format,
 			   NULL, &xii->shm, image->width, image->height);
@@ -133,6 +136,7 @@ static void createImage(XImageInfo *xii,
 		}
 		XDestroyImage(xii->ximage);
 	}
+#endif
 
 	xii->ximage = XCreateImage(xii->disp, visual, depth, format,
 		0, NULL, image->width, image->height, 8, 0);
@@ -176,7 +180,9 @@ XImageInfo *imageToXImage(Display *disp,
   xii->foreground= xii->background= 0;
   xii->gc= NULL;
   xii->ximage= NULL;
+#ifndef NO_XSHM
   xii->shm.shmid = -1;
+#endif
 
   /* process image based on type of visual we're sending to */
 
@@ -687,7 +693,7 @@ XImageInfo *imageToXImage(Display *disp,
 	byte *src;
 
 	src = image->data;
-        for (y = 0; y < image->height; ++x) {
+        for (y = 0; y < image->height; ++y) {
	  for (x = 0; x < image->width; ++x) {
	    XPutPixel(xii->ximage, x, y,
	      xii->index[c + memToVal(src, image->pixlen)]);
@@ -761,11 +767,14 @@ void sendXImage(XImageInfo *xii, int src
       src_x + w > xii->ximage->width || src_y + h > xii->ximage->height)
     return;
 
+#ifndef NO_XSHM
   if (xii->shm.shmid >= 0) {
     XShmPutImage(xii->disp, xii->drawable, xii->gc,
       xii->ximage, src_x, src_y, dst_x, dst_y, w, h, False);
     XSync(xii->disp, False);
-  } else {
+  } else
+#endif
+  {
     XPutImage(xii->disp, xii->drawable, xii->gc,
       xii->ximage, src_x, src_y, dst_x, dst_y, w, h);
   }
@@ -783,11 +792,14 @@ void freeXImage(Image *image, XImageInfo
   }
   if (xii->gc)
     XFreeGC(xii->disp, xii->gc);
+#ifndef NO_XSHM
   if (xii->shm.shmid >= 0) {
     XShmDetach(xii->disp, &xii->shm);
     shmdt(xii->shm.shmaddr);
     shmctl(xii->shm.shmid, IPC_RMID, 0);
-  } else {
+  } else
+#endif
+  {
     lfree((byte *) xii->ximage->data);
   }
   xii->ximage->data= NULL;
