$NetBSD$

Teach gnulib's getprogname replacement to obtain the current image name from
OpenVMS.  LIB$GETJPI returns an OpenVMS file specification, so strip its
directory, type, and version before returning it.

--- lib/getprogname.c.orig	2025-01-03 11:36:00.000000000 +0000
+++ lib/getprogname.c
@@ -21,6 +21,13 @@
 
 #include <errno.h> /* get program_invocation_name declaration */
 
+#ifdef __VMS
+# include <descrip.h>
+# include <jpidef.h>
+# include <lib$routines.h>
+# include <string.h>
+#endif
+
 #ifdef _AIX
 # include <unistd.h>
 # include <procinfo.h>
@@ -91,6 +98,51 @@ getprogname (void)
 #  else
   return p && p[0] ? p : "?";
 #  endif
+# elif defined __VMS                                    /* OpenVMS */
+  static char image[1024];
+  static char *p;
+  static int initialized;
+
+  if (!initialized)
+    {
+      unsigned short length = 0;
+      int item = JPI$_IMAGNAME;
+      unsigned int status;
+      struct dsc$descriptor_s descriptor =
+        {
+          sizeof image - 1,
+          DSC$K_DTYPE_T,
+          DSC$K_CLASS_S,
+          image
+        };
+
+      initialized = 1;
+      p = "?";
+      status = lib$getjpi (&item, 0, 0, 0, &descriptor, &length);
+      if ((status & 1) && length < sizeof image)
+        {
+          char *end;
+
+          image[length] = '\0';
+          p = strrchr (image, ']');
+          if (p)
+            p++;
+          else
+            {
+              p = strrchr (image, ':');
+              p = p ? p + 1 : image;
+            }
+          end = strchr (p, ';');
+          if (end)
+            *end = '\0';
+          end = strrchr (p, '.');
+          if (end)
+            *end = '\0';
+          if (!p[0])
+            p = "?";
+        }
+    }
+  return p;
 # elif _AIX                                                 /* AIX */
   /* Idea by Bastien ROUCARIÈS,
      https://lists.gnu.org/r/bug-gnulib/2010-12/msg00095.html
