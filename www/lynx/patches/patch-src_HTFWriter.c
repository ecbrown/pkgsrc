$NetBSD$

Use VSI's system-service declarations so external names retain their native
spelling when GNV builds with case-sensitive external names.

--- src/HTFWriter.c.orig	2024-05-27 17:02:24.000000000 +0000
+++ src/HTFWriter.c
@@ -1550,6 +1550,7 @@
 #include <iodef.h>		/* I/O function codes */
 #include <fibdef.h>		/* file information block defs */
 #include <atrdef.h>		/* attribute request codes */
+#include <starlet.h>		/* system-service prototypes */
 #ifdef NOTDEFINED /*** Not all versions of VMS compilers have these.	 ***/
 #include <fchdef.h>		/* file characteristics */
 #include <fatdef.h>		/* file attribute defs */
@@ -1575,8 +1576,6 @@ typedef struct dsc {
     void *adr;
 } Desc;
 
-extern unsigned long sys$open(), sys$qiow(), sys$dassgn();
-
 #define syswork(sts)	((sts) & 1)
 #define sysfail(sts)	(!syswork(sts))
 
