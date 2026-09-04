$NetBSD$

Use the pkgsrc installation paths for shared data and help instead of the
historical sys$login location on current OpenVMS systems.  The placeholders
are replaced by the package Makefile before configure runs.

--- src/syscfg.h.orig	2025-10-21 17:28:21.000000000 +0000
+++ src/syscfg.h
@@ -75,12 +75,9 @@
 # endif
 # define HOME   "sys$login"
 # define PLOTRC "gnuplot.ini"
-# ifdef NO_GIH
-   /* for show version long */
-#  define HELPFILE "GNUPLOT$HELP"
-# else
-#  define HELPFILE "sys$login:gnuplot.gih"
-# endif
+# define GNUPLOT_SHARE_DIR "@PREFIX@/share/gnuplot/@API_VERSION@"
+# define GNUPLOT_PS_DIR "@PREFIX@/share/gnuplot/@API_VERSION@/PostScript"
+# define HELPFILE "@PREFIX@/share/gnuplot/@API_VERSION@/gnuplot.gih"
 # if !defined(VAXCRTL) && !defined(DECCRTL)
 #  define VAXCRTL VAXCRTL_AND_DECCRTL_UNDEFINED
 #  define DECCRTL VAXCRTL_AND_DECCRTL_UNDEFINED
