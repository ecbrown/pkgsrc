$NetBSD$

OpenVMS provides termios declarations but not the tcgetattr/tcsetattr
entry points.  Leave terminal input in its normal canonical mode and use
the C RTL input routine; this retains gnuplot's built-in history and line
editing code without creating executables containing unresolved symbols.

--- src/readline.c.orig
+++ src/readline.c
@@ -221,7 +221,7 @@
 # endif
 #endif /* not HAVE_TERMIOS_H && HAVE_TCGETATTR */
 
-#if !defined(MSDOS) && !defined(_WIN32)
+#if !defined(MSDOS) && !defined(_WIN32) && !defined(__VMS)
 
 /*
  * Set up structures using the proper include file
@@ -311,6 +311,11 @@
 
 #else /* MSDOS or _WIN32 */
 
+# ifdef __VMS
+#  define special_getc() getchar()
+#  define DEL_ERASES_CURRENT_CHAR
+# endif
+
 # ifdef _WIN32
 #  include <windows.h>
 #  include "win/winmain.h"
@@ -1449,7 +1454,7 @@
     cur_pos = max_pos = strlen(cur_line);
 }
 
-#if !defined(MSDOS) && !defined(_WIN32)
+#if !defined(MSDOS) && !defined(_WIN32) && !defined(__VMS)
 /* Convert ANSI arrow keys to control characters */
 static int
 ansi_getc()
@@ -1626,7 +1631,7 @@
 void
 set_termio()
 {
-#if !defined(MSDOS) && !defined(_WIN32)
+#if !defined(MSDOS) && !defined(_WIN32) && !defined(__VMS)
 /* set termio so we can do our own input processing */
 /* and save the old terminal modes so we can reset them later */
     if (term_set == 0) {
@@ -1736,7 +1741,7 @@
 void
 reset_termio()
 {
-#if !defined(MSDOS) && !defined(_WIN32)
+#if !defined(MSDOS) && !defined(_WIN32) && !defined(__VMS)
 /* reset saved terminal modes */
     if (term_set == 1) {
 #  ifdef SGTTY
