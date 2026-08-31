$NetBSD$

The OpenVMS backend passes an unparsed command-line string to its DCL parser,
not a Unix argv vector.  Match the implementation and its callers.  Keep the
spawned process ID in the shared child prefix that the backend receives.

--- src/job.h.orig	2023-02-20 19:46:52.000000000 +0000
+++ src/job.h
@@ -23,7 +23,8 @@ Copyright (C) 1992-2023 Free Software Fo
     char *comname;              /* Temporary command file name */       \
     int efn;                    /* Completion event flag number */      \
     int cstatus;                /* Completion status */                 \
-    int vms_launch_status;      /* non-zero if lib$spawn, etc failed */
+    int vms_launch_status;      /* non-zero if lib$spawn, etc failed */  \
+    pid_t pid;                  /* Child process ID returned by spawn */
 #else
 #define VMSCHILD
 #endif
@@ -54,7 +55,9 @@ struct child
 
     unsigned int  command_line; /* Index into command_lines.  */
 
+#ifndef VMS
     pid_t pid;                  /* Child process's ID number.  */
+#endif
 
     unsigned int  remote:1;     /* Nonzero if executing remotely.  */
     unsigned int  noerror:1;    /* Nonzero if commands contained a '-'.  */
@@ -81,1 +81,5 @@
+#ifdef VMS
+pid_t child_execute_job (struct childbase *child, int good_stdin, char *argv);
+#else
 pid_t child_execute_job (struct childbase *child, int good_stdin, char **argv);
+#endif
