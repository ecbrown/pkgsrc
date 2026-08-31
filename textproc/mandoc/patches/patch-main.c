$NetBSD$

OpenVMS does not provide fchdir(2), fork(2), or the process-group terminal
interfaces used by mandoc's interactive pager.  File formatting and man-page
searching remain available; disable only the optional pager path and the
best-effort directory restoration.

--- main.c.orig	2021-09-04 22:38:46.000000000 +0000
+++ main.c
@@ -111,7 +111,9 @@
 static	void		  process_onefile(struct mparse *, struct manpage *,
 				int, struct outstate *, struct manconf *);
 static	void		  run_pager(struct outstate *, char *);
+#ifndef __VMS
 static	pid_t		  spawn_pager(struct outstate *, char *);
+#endif
 static	void		  usage(enum argmode) __attribute__((__noreturn__));
 static	int		  woptions(char *, enum mandoc_os *, int *);
 
@@ -210,6 +212,10 @@
 	outst.tag_files = NULL;
 	outst.outtype = OUTT_LOCALE;
 	outst.use_pager = 1;
+#ifdef __VMS
+	/* The OpenVMS C RTL has no fork(2) or controlling-terminal pgrp API. */
+	outst.use_pager = 0;
+#endif
 
 	show_usage = 0;
 	outmode = OUTMODE_DEF;
@@ -621,7 +627,9 @@
 			break;
 	}
 	if (startdir != -1) {
+#ifndef __VMS
 		(void)fchdir(startdir);
+#endif
 		close(startdir);
 	}
 	if (conf.output.tag != NULL && conf.output.tag_found == 0) {
@@ -879,7 +887,9 @@
 	if (resp->ipath != SIZE_MAX)
 		(void)chdir(conf->manpath.paths[resp->ipath]);
 	else if (startdir != -1)
+#ifndef __VMS
 		(void)fchdir(startdir);
+#endif
 
 	mandoc_msg_setinfilename(resp->file);
 	if (resp->file != NULL) {
@@ -1213,6 +1223,7 @@
 	return 0;
 }
 
+#ifndef __VMS
 /*
  * Wait until moved to the foreground,
  * then fork the pager and wait for the user to close it.
@@ -1373,3 +1384,11 @@
 	mandoc_msg(MANDOCERR_EXEC, 0, 0, "%s: %s", argv[0], strerror(errno));
 	_exit(mandoc_msg_getrc());
 }
+#else
+static void
+run_pager(struct outstate *outst, char *tag_target)
+{
+	(void)outst;
+	(void)tag_target;
+}
+#endif
