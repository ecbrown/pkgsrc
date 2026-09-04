$NetBSD: patch-src_plot.c,v 1.3 2025/01/05 09:08:55 adam Exp $

NetBSD editline keeps header files in "readline", not "editline".
Same applies to DragonFly.

The GNV POSIX input path does not use an SMG virtual keyboard.

--- src/plot.c.orig	2025-10-21 17:28:21.000000000 +0000
+++ src/plot.c
@@ -81,7 +81,7 @@
 /* BSD editline
 */
 #ifdef HAVE_LIBEDITLINE
-#  include <editline/readline.h>
+#  include <readline/readline.h>
 #  include <histedit.h>
 #endif
 
@@ -444,7 +444,7 @@ main(int argc, char **argv)
-	}
+	}
     }
-
+
-#ifdef VMS
+#if defined(VMS) && !defined(GP_OPENVMS_POSIX)
     vms_init_screen();
 #endif /* VMS */
-
+
