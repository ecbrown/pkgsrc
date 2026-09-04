$NetBSD$

VSI C requires complex.h to precede any header which includes math.h;
otherwise math.h exposes the legacy struct-based cabs declarations.

--- src/complexfun.c.orig	2025-10-21 17:28:21.000000000 +0000
+++ src/complexfun.c
@@ -56,6 +56,10 @@
  *
 ]*/
 
+#if defined(vms) || defined(VMS) || defined(__VMS)
+# include <complex.h>
+#endif
+
 #include "syscfg.h"
 #include "gp_types.h"
 #include "eval.h"
