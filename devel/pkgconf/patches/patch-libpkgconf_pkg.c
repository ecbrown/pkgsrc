$NetBSD$

OpenVMS does not expose the POSIX S_ISLNK macro.  Skip the optional symlink
resolution loop when deriving a package file's parent directory.

--- libpkgconf/pkg.c.orig	2026-08-23 18:53:18.000000000 +0000
+++ libpkgconf/pkg.c
@@ -65,7 +65,7 @@ pkg_get_parent_dir(pkgconf_pkg_t *pkg)
 	if (!pkgconf_buffer_append(&buf, pkg->filename))
 		goto fail;
 
-#ifndef _WIN32
+#if !defined(_WIN32) && !defined(__VMS)
 	struct stat path_stat;
 
 	while (buf.base != NULL &&
