$NetBSD$

Declare the POSIX terminal routines supplied by the OpenVMS compatibility
code.  VSI provides the termios types but leaves these routines unimplemented.

--- include/nc_termios.h.orig	2025-10-18 17:53:13.000000000 +0000
+++ include/nc_termios.h
@@ -38,6 +38,16 @@
 
 #include <ncurses_cfg.h>
 
+#ifdef __VMS
+extern int tcgetattr(int, struct termios *);
+extern int tcsetattr(int, int, const struct termios *);
+extern int tcflush(int, int);
+extern speed_t cfgetispeed(const struct termios *);
+extern speed_t cfgetospeed(const struct termios *);
+extern int cfsetispeed(struct termios *, speed_t);
+extern int cfsetospeed(struct termios *, speed_t);
+#endif
+
 #if HAVE_TERMIOS_H && HAVE_TCGETATTR
 
 #else /* !HAVE_TERMIOS_H */
