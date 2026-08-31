$NetBSD$

Expose the native OpenVMS input helpers to ncurses' internal modules.

--- ncurses/curses.priv.h.orig	2025-12-27 21:46:04.000000000 +0000
+++ ncurses/curses.priv.h
@@ -480,6 +480,11 @@
 
 #include <nc_termios.h>
 
+#ifdef __VMS
+extern NCURSES_EXPORT(ssize_t) _nc_vms_read(int, void *, size_t);
+extern NCURSES_EXPORT(int) _nc_vms_typeahead(int);
+#endif
+
 #define IsPreScreen(sp)      (((sp) != NULL) && sp->_prescreen)
 #define HasTerminal(sp)      (((sp) != NULL) && (NULL != ((sp)->_term)))
 #define IsValidScreen(sp)    (HasTerminal(sp) && !IsPreScreen(sp))
