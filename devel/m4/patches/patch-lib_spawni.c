$NetBSD$

OpenVMS provides vfork(2) but not fork(2), and its vfork has intentionally
different semantics: code between vfork and exec runs in the parent.  Translate
the standard-stream actions needed by gnulib's pipe helpers to the native
decc$set_child_standard_streams interface, and do not mutate the parent while
preparing the child.

--- lib/spawni.c.orig	2025-05-17 08:45:21.000000000 +0000
+++ lib/spawni.c
@@ -40,6 +40,10 @@
 #include <string.h>
 #include <unistd.h>
 
+#ifdef __VMS
+extern int decc$set_child_standard_streams (int, int, int);
+#endif
+
 #if _LIBC
 # include <not-cancel.h>
 #else
@@ -868,6 +872,73 @@
   /* Do this once.  */
   short int flags = attrp == NULL ? 0 : attrp->_flags;
 
+#ifdef __VMS
+  int child_stdin = -1;
+  int child_stdout = -1;
+  int child_stderr = -1;
+
+  /* OpenVMS executes the code between vfork and exec in the parent.  Its
+     native child-stream mapping supplies the standard-descriptor actions
+     without changing the parent's descriptor table.  */
+  if ((flags & ~POSIX_SPAWN_SETSIGMASK) != 0)
+    return ENOSYS;
+  if (file_actions != NULL)
+    for (int cnt = 0; cnt < file_actions->_used; ++cnt)
+      {
+        const struct __spawn_action *action = &file_actions->_actions[cnt];
+
+        switch (action->tag)
+          {
+          case spawn_do_dup2:
+            switch (action->action.dup2_action.newfd)
+              {
+              case STDIN_FILENO:
+                child_stdin = action->action.dup2_action.fd;
+                break;
+              case STDOUT_FILENO:
+                child_stdout = action->action.dup2_action.fd;
+                break;
+              case STDERR_FILENO:
+                child_stderr = action->action.dup2_action.fd;
+                break;
+              default:
+                return ENOSYS;
+              }
+            break;
+
+          case spawn_do_close:
+            /* The pipe helpers mark their nonstandard descriptors
+               close-on-exec.  Closing them here would affect the parent.  */
+            if (action->action.close_action.fd <= STDERR_FILENO)
+              return ENOSYS;
+            break;
+
+          default:
+            return ENOSYS;
+          }
+      }
+  if (file_actions != NULL)
+    for (int cnt = 0; cnt < file_actions->_used; ++cnt)
+      {
+        const struct __spawn_action *action = &file_actions->_actions[cnt];
+        if (action->tag == spawn_do_close)
+          {
+            int fd = action->action.close_action.fd;
+
+            if (fd != child_stdin && fd != child_stdout && fd != child_stderr)
+              {
+                int fd_flags = fcntl (fd, F_GETFD);
+                if (fd_flags < 0 || fcntl (fd, F_SETFD,
+                                            fd_flags | FD_CLOEXEC) < 0)
+                  return errno;
+              }
+          }
+      }
+  if (decc$set_child_standard_streams (child_stdin, child_stdout,
+                                       child_stderr) < 0)
+    return errno;
+#endif
+
   /* Avoid gcc warning
        "variable 'flags' might be clobbered by 'longjmp' or 'vfork'"  */
   (void) &flags;
@@ -912,20 +983,29 @@
 
   /* Generate the new process.  */
   pid_t new_pid;
-#if HAVE_VFORK
+#if defined __VMS
+  new_pid = vfork ();
+#elif HAVE_VFORK
   if ((flags & POSIX_SPAWN_USEVFORK) != 0
       /* If no major work is done, allow using vfork.  */
       || ((flags & (POSIX_SPAWN_SETSIGMASK | POSIX_SPAWN_SETSIGDEF
                     | POSIX_SPAWN_SETSCHEDPARAM | POSIX_SPAWN_SETSCHEDULER
                     | POSIX_SPAWN_SETPGROUP | POSIX_SPAWN_RESETIDS)) == 0
           && file_actions == NULL))
     new_pid = vfork ();
   else
-#endif
     new_pid = fork ();
+#else
+  new_pid = fork ();
+#endif
 
   if (new_pid != 0)
     {
+#ifdef __VMS
+      /* The mapping remains active until explicitly reset.  At this point
+         exec has captured it for the new subprocess.  */
+      decc$set_child_standard_streams (-1, -1, -1);
+#endif
       if (new_pid < 0)
         return errno;
 
@@ -936,6 +1016,7 @@
       return 0;
     }
 
+#ifndef __VMS
   /* Set signal mask.  */
   if ((flags & POSIX_SPAWN_SETSIGMASK) != 0
       && sigprocmask (SIG_SETMASK, &attrp->_ss, NULL) != 0)
@@ -1049,6 +1130,7 @@
             break;
           }
       }
+#endif
 
   if (! use_path)
     {
