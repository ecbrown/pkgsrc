$NetBSD$

Use ncurses' chtype and attribute operations on OpenVMS when ncurses is the
selected screen library.

--- src/LYCurses.h.orig	2023-10-23 23:35:36.000000000 +0000
+++ src/LYCurses.h
@@ -130,7 +130,7 @@ typedef unsigned int chtype;
 #define HAVE_TYPE_CHTYPE 1
 #endif
 
-#if defined(_VMS_CURSES) || defined(VMS)
+#if defined(_VMS_CURSES) || (defined(VMS) && !defined(NCURSES))
 typedef char chtype;
 
 #define HAVE_TYPE_CHTYPE 1
@@ -649,7 +649,7 @@ extern int lynx_chg_color(int, int);
 #ifdef FANCY_CURSES
 #define SHOW_WHEREIS_TARGETS 1
 
-#ifdef VMS
+#if defined(VMS) && !defined(NCURSES)
 /*
  *  For VMS curses, [w]setattr() and [w]clrattr()
  *  add and subtract, respectively, the attributes
@@ -708,7 +708,7 @@ extern int lynx_chg_color(int, int);
 #define stop_reverse()		LYsubAttr(A_REVERSE)
 #define wstop_reverse(w)	LYsubWAttr(w, A_REVERSE)
 
-#endif				/* VMS */
+#endif				/* VMS && !NCURSES */
 
 #else				/* Not FANCY_CURSES: */
 /* *INDENT-OFF* */
