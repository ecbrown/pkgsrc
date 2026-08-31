$NetBSD$

Avoid the ncurses typeahead symbol on OpenVMS and use VSI's MAIL routine
declarations so GNV's case-sensitive linker sees the native symbol spelling.

--- src/LYUtils.c.orig	2024-01-15 17:10:52.000000000 +0000
+++ src/LYUtils.c
@@ -1871,7 +1871,7 @@ int HTCheckForInterrupt(void)
 #endif
 
 #else /* VMS: */
-    extern int typeahead(void);
+    extern int lynx_typeahead(void);
 
     /** Control-C or Control-Y and a 'N'o reply to exit query **/
     if (HadVMSInterrupt) {
@@ -1879,7 +1879,7 @@ int HTCheckForInterrupt(void)
 	return ((int) TRUE);
     }
 
-    c = typeahead();
+    c = lynx_typeahead();
 
 #endif /* !VMS */
 
@@ -3919,6 +3919,7 @@ void print_restrictions_to_fd(FILE *fp)
 #ifdef VMS
 #include <jpidef.h>
 #include <maildef.h>
+#include <mail$routines.h>
 #include <starlet.h>
 
 typedef struct _VMSMailItemList {
@@ -3950,10 +3951,6 @@ void LYCheckMail(void)
 		     {0,0,0,0}};
     /* *INDENT-ON* */
 
-    extern long mail$user_begin();
-    extern long mail$user_get_info();
-    extern long mail$user_end();
-
     if (failure)
 	return;
 
