$NetBSD$

Add the missing VMS MMS action for the generated RealPPPort object.

--- dist/Devel-PPPort/Makefile.PL.orig
+++ dist/Devel-PPPort/Makefile.PL
@@ -185,6 +185,10 @@
 regen: regen_pm regen_xs regen_tests regen_h regen_release_date
 
 POSTAMBLE
+  if ($^O eq 'VMS') {
+    $post .= "\nRealPPPort.obj : RealPPPort.c\n";
+    $post .= "\t\\\$(CCCMD) \\\$(CCCDLFLAGS) RealPPPort.c /OBJECT=RealPPPort.obj\n";
+  }
   return $post;
 }
 
