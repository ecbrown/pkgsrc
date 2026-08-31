$NetBSD$

The OpenVMS recipe path uses its command string directly and never constructs
a Unix argv vector.  Provide the unused public wrapper with a local stub for
the generic, OpenVMS-excluded argument parser.

--- src/job.c.orig	2022-10-31 06:23:04.000000000 +0000
+++ src/job.c
@@ -3703,6 +3703,18 @@ construct_command_argv_internal (char *l
 }
 #endif /* !VMS */
 
+#ifdef VMS
+static char **
+construct_command_argv_internal (char *line UNUSED, char **restp UNUSED,
+                                 const char *shell UNUSED,
+                                 const char *shellflags UNUSED,
+                                 const char *ifs UNUSED, int flags UNUSED,
+                                 char **batch_filename UNUSED)
+{
+  return 0;
+}
+#endif
+
 /* Figure out the argument list necessary to run LINE as a command.  Try to
    avoid using a shell.  This routine handles only ' quoting, and " quoting
    when no backslash, $ or ' characters are seen in the quotes.  Starting
