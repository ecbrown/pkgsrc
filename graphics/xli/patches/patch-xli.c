$NetBSD$

OpenVMS has no fork().  The -fork option is nonessential, so diagnose it
instead of retaining an unresolved reference in the executable.

--- xli.c.orig	2005-02-28 01:42:39.000000000 +0100
+++ xli.c
@@ -244,6 +244,7 @@ int main(int argc, char *argv[])
 		exit(1);
 	}
 
+#ifndef VMS
 	if (globals.do_fork) {
 		switch (fork()) {
 		case -1:
@@ -255,6 +256,10 @@ int main(int argc, char *argv[])
 			exit(0);
 		}
 	}
+#else
+	if (globals.do_fork)
+		fprintf(stderr, "%s: -fork is not available on OpenVMS\n", globals.argv0);
+#endif
 
 	/* -default: resets colormap and load default root weave */
 
