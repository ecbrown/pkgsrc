$NetBSD$

Give MMS the generated C dependency and a no-op static target.  Without the
explicit dependency, VMS MMS may try to link DynaLoader before DynaLoader.c
exists.

--- ext/DynaLoader/Makefile.PL.orig
+++ ext/DynaLoader/Makefile.PL
@@ -15,7 +15,8 @@
     VERSION_FROM => 'DynaLoader_pm.PL',
     PL_FILES	=> {'DynaLoader_pm.PL'=>'DynaLoader.pm'},
     PM		=> {'DynaLoader.pm' => '$(INST_LIBDIR)/DynaLoader.pm'},
-    depend      => {	'DynaLoader$(OBJ_EXT)' => 'dlutils.c',
+    depend      => {	'DynaLoader$(OBJ_EXT)' =>
+                    ($is_vms ? 'DynaLoader.c dlutils.c' : 'dlutils.c'),
 			'DynaLoader.c' => 'DynaLoader.xs'},
     clean	=> {FILES => 'DynaLoader.c DynaLoader.xs DynaLoader.pm'},
 );
@@ -70,5 +71,6 @@
 	\$(CP) \$(OBJECT) $object
 
 static :: $object
+	\$(NOECHO) \$(NOOP)
 ";
 }
