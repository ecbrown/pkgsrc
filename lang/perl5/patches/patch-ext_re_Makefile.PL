$NetBSD$

Use a VMS upward directory specification for the generated regex source
dependencies; the POSIX spelling is rejected by MMS.

--- ext/re/Makefile.PL.orig
+++ ext/re/Makefile.PL
@@ -52,6 +52,7 @@ WriteMakefile(
 
 package MY;
 sub upupfile {
+    return '[--]' . $_[0] if $^O eq 'VMS';
     File::Spec->catfile(File::Spec->updir, File::Spec->updir, $_[0]);
 }
