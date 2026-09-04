$NetBSD$

VSI C does not accept an unnamed parameter in a function definition.

--- src/multiplot.c.orig	2025-10-21 17:28:21.000000000 +0000
+++ src/multiplot.c
@@ -992,7 +992,11 @@ restore_panel_axis_mappings(int p)
 
 #else /* USE_MOUSE */
 
-void set_panel_flag(unsigned int) {}
+void
+set_panel_flag(unsigned int flags)
+{
+    (void) flags;
+}
 void restore_panel_axis_mappings() {}
 
 #endif /* USE_MOUSE */
