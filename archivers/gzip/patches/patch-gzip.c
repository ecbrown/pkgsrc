$NetBSD$

OpenVMS RMS record-mode redirection can append one line-feed record
terminator after a binary stream.  Accept only that exact final byte after
a valid gzip member, while retaining errors for truncated headers and support
for concatenated gzip members.

--- gzip.c.orig	2025-02-24 03:16:09.000000000 +0000
+++ gzip.c
@@ -1466,7 +1466,19 @@ get_method (int in)
         magic[0] = get_byte ();
         imagic0 = 0;
         if (magic[0]) {
+#ifdef __VMS
+            imagic1 = try_byte ();
+            if (imagic1 == EOF) {
+                /* RMS redirection can append a line-feed record
+                   terminator after an otherwise complete member.  */
+                if (part_nb > 0 && magic[0] == '\n')
+                    return -3;
+                read_error ();
+            }
+            magic[1] = imagic1;
+#else
             magic[1] = get_byte ();
+#endif
             imagic1 = 0; /* avoid lint warning */
         } else {
             imagic1 = try_byte ();
