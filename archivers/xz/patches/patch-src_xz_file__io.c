$NetBSD$

OpenVMS x86-64 has scalar st_ino and st_dev fields.  The array-shaped st_ino
handling is for the older OpenVMS stat layout.

--- src/xz/file_io.c.orig	2026-03-30 11:42:14.000000000 +0000
+++ src/xz/file_io.c
@@ -278 +278 @@ io_unlink(const char *name, const struct stat *known_st)
-#	ifdef __VMS
+#	if defined(__VMS) && !defined(__x86_64__)
@@ -989 +989 @@ io_open_dest_real(file_pair *pair)
-#if defined(__VMS)
+#if defined(__VMS) && !defined(__x86_64__)
