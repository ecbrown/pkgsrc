$NetBSD$

Use a native QIO for single-character terminal input on OpenVMS.  The C RTL
read routine remains record-oriented in pass-through mode.

--- ncurses/base/lib_getch.c.orig	2025-12-27 12:28:45.000000000 +0000
+++ ncurses/base/lib_getch.c
@@ -314,8 +314,12 @@
 			     &buf);
 	c2 = buf;
 #else
+#ifdef __VMS
+	n = (int) _nc_vms_read(sp->_ifd, &c2, (size_t) 1);
+#else
 	n = (int) read(sp->_ifd, &c2, (size_t) 1);
 #endif
+#endif
 	_nc_set_read_thread(FALSE);
 	ch = c2;
 #endif /* USE_TERM_DRIVER */
