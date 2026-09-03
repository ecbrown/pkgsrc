$NetBSD$

The OpenVMS C RTL keeps terminal reads record-oriented even when the terminal
is in pass-through mode.  Use the single-character QIO wrapper supplied by
pkgsrc ncurses, which is already the termcap provider for this package.

Do not probe look(1) on OpenVMS, where it is absent and exec failure reporting
has platform-specific semantics.  Fall back directly to grep.

--- config.X.orig
+++ config.X
@@ -707,8 +707,10 @@
 /* look command (if look(1) MAY BE available - ignored if not) */
 #ifndef REGEX_LOOKUP
 #ifndef LOOK
+#ifndef __VMS
 #define	LOOK	"look -df"
 #endif
+#endif
 #endif /* REGEX_LOOKUP */
 
 /* path to egrep (use speeded up version if available) */
@@ -1166,7 +1168,11 @@
 ** getchar() doesn't operate properly in raw mode.
 */
 #ifndef GETKEYSTROKE
+#ifdef __VMS
+#define GETKEYSTROKE() vms_get_keystroke ()
+#else
 #define GETKEYSTROKE()	getchar ()
+#endif
 #endif /* GETKEYSTROKE */
 
 /*
