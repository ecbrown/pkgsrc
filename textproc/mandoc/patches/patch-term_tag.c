$NetBSD$

OpenVMS has no tcgetpgrp(3) or tcsetpgrp(3).  Pager process-group cleanup is
already disabled with the pager itself, so omit that optional cleanup code.

--- term_tag.c.orig	2021-03-30 17:16:55.000000000 +0000
+++ term_tag.c
@@ -192,8 +192,11 @@
 void
 term_tag_unlink(void)
 {
+#ifndef __VMS
 	pid_t	 tc_pgid;
+#endif
 
+#ifndef __VMS
 	if (tag_files.tcpgid != -1) {
 		tc_pgid = tcgetpgrp(STDOUT_FILENO);
 		if (tc_pgid == tag_files.pager_pid ||
@@ -201,6 +204,7 @@
 		    getpgid(tc_pgid) == -1)
 			(void)tcsetpgrp(STDOUT_FILENO, tag_files.tcpgid);
 	}
+#endif
 	if (strncmp(tag_files.ofn, "/tmp/man.", 9) == 0) {
 		unlink(tag_files.ofn);
 		*tag_files.ofn = '\0';
