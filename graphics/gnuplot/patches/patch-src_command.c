$NetBSD$

GNV supplies working POSIX stdio, system(3), and popen(3) interfaces.  Prefer
those over the legacy SMG and LIB$ paths, whose lowercase external names do
not match the case-sensitive x86-64 shareable-image symbols.

--- src/command.c.orig	2025-10-21 17:28:21.000000000 +0000
+++ src/command.c
@@ -3246,7 +3246,7 @@ replotrequest()
 
 /* Support for input, shell, and help for various systems */
 
-#ifdef VMS
+#if defined(VMS) && !defined(GP_OPENVMS_POSIX)
 
 # include <descrip.h>
 # include <rmsdef.h>
@@ -3624,7 +3624,7 @@ help_command()
 }
 #endif /* !NO_GIH */
 
-#ifndef VMS
+#if !defined(VMS) || defined(GP_OPENVMS_POSIX)
 
 static void
 do_system(const char *cmd)
@@ -4106,14 +4106,14 @@ do_system_func(const char *cmd, char **o
     int result_allocated, result_pos;
     char* result;
     int ierr = 0;
-# if defined(VMS)
+# if defined(VMS) && !defined(GP_OPENVMS_POSIX)
     int chan, one = 1;
     struct dsc$descriptor_s pgmdsc = {0, DSC$K_DTYPE_T, DSC$K_CLASS_S, 0};
     static $DESCRIPTOR(lognamedsc, "PLOT$MAILBOX");
 # endif /* VMS */
 
     /* open stream */
-# ifdef VMS
+# if defined(VMS) && !defined(GP_OPENVMS_POSIX)
     pgmdsc.dsc$a_pointer = cmd;
     pgmdsc.dsc$w_length = strlen(cmd);
     if (!((vaxc$errno = sys$crembx(0, &chan, 0, 0, 0, 0, &lognamedsc)) & 1))
