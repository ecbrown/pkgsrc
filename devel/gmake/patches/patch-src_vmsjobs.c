$NetBSD$

The VSI C RTL has sigblock() and sigsetmask() but no siggetmask().  The generic
job module therefore uses its no-op signal wrapper, while the VMS backend still
blocks the fatal-signal mask directly.  Declare the mask it shares with main.c.

When make is running from GNV Bash, execute each recipe as a temporary Bash
script.  Upstream detects this mode but still parses the recipe as DCL, so a
normal pkgsrc command beginning with a Unix path (for example /bin/echo) is
submitted to DCL as an empty command.  A script also preserves shell quoting
without trying to quote an arbitrary recipe through a DCL command line.

--- src/vmsjobs.c.orig	2023-02-20 19:46:52.000000000 +0000
+++ src/vmsjobs.c
@@ -833,6 +835,7 @@ child_execute_job (struct childbase *chil
   /* Parse IO redirection.  */
 
   child->comname = NULL;
+  token.use_cmd_file = 0;
 
   DB (DB_JOBS, ("child_execute_job (%s)\n", argv));
 
@@ -857,6 +860,52 @@ child_execute_job (struct childbase *chil
   pnamedsc.dsc$b_dtype = DSC$K_DTYPE_T;
   pnamedsc.dsc$b_class = DSC$K_CLASS_S;
 
+  /* The VMS parser below translates shell syntax to DCL.  That is useful for
+     native makefiles, but GNV makefiles need to run in the shell they were
+     written for.  Put the complete recipe in a file so neither DCL nor an
+     additional shell -c layer changes its quoting.  */
+  if (vms_gnv_shell)
+    {
+      FILE *outfile;
+      size_t argv_len = strlen (argv);
+      int write_failed = 0;
+
+      outfile = get_tmpfile (&child->comname);
+      if (outfile == NULL)
+        {
+          child->cstatus = VMS_POSIX_EXIT_MASK | (MAKE_TROUBLE << 3);
+          child->vms_launch_status = SS$_ABORT;
+          child->efn = 0;
+          return -1;
+        }
+
+      if (fwrite (argv, 1, argv_len, outfile) != argv_len)
+        write_failed = 1;
+      if (argv[argv_len - 1] != '\n' && fputc ('\n', outfile) == EOF)
+        write_failed = 1;
+      if (fclose (outfile) == EOF)
+        write_failed = 1;
+      if (write_failed)
+        {
+          unlink (child->comname);
+          free (child->comname);
+          child->comname = NULL;
+          child->cstatus = VMS_POSIX_EXIT_MASK | (MAKE_TROUBLE << 3);
+          child->vms_launch_status = SS$_ABORT;
+          child->efn = 0;
+          return -1;
+        }
+
+      cmd_dsc = xmalloc (sizeof (struct dsc$descriptor_s));
+      cmd_dsc->dsc$a_pointer = xmalloc (strlen (child->comname) + 8);
+      sprintf (cmd_dsc->dsc$a_pointer, "$ bash %s", child->comname);
+      cmd_dsc->dsc$w_length = strlen (cmd_dsc->dsc$a_pointer);
+      cmd_dsc->dsc$b_dtype = DSC$K_DTYPE_T;
+      cmd_dsc->dsc$b_class = DSC$K_CLASS_S;
+      DB (DB_JOBS, (_("Bash: %s\n"), cmd_dsc->dsc$a_pointer));
+      goto command_ready;
+    }
+
   /* Old */
   /* Handle comments and redirection.
      For ONESHELL, the redirection must be on the first line. Any other
@@ -1321,6 +1370,7 @@ child_execute_job (struct childbase *chil
       DB (DB_JOBS, (_("Executing %s instead\n"), child->comname));
     }
 
+ command_ready:
   child->efn = 0;
   while (child->efn < 32 || child->efn > 63)
     {
@@ -1421,7 +1471,8 @@ child_execute_job (struct childbase *chil
 #endif
 
   /* Free the pointer if not a command file */
-  if (!vms_always_use_cmd_file && !token.use_cmd_file)
+  if (vms_gnv_shell
+      || (!vms_always_use_cmd_file && !token.use_cmd_file))
     free (cmd_dsc->dsc$a_pointer);
   free (cmd_dsc);
 
