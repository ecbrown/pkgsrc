$NetBSD$

Use a standard C99 variadic macro.  VSI C supports variadic macros but does
not implement GCC's empty-argument comma elision extension.

--- cli/bomtool/main.c.orig	2023-10-16 08:13:22.000000000 +0000
+++ cli/bomtool/main.c
@@ -42,17 +42,17 @@ static int maximum_traverse_depth = 2000
 static FILE *error_msgout = NULL;
 static FILE *sbom_out = NULL;
 
-#define OUTPUT_OR_RET(client, f, fmt, ...) \
+#define OUTPUT_OR_RET(client, f, ...) \
 	do { \
-		if (!pkgconf_output_file_fmt((f), (fmt), ##__VA_ARGS__)) { \
+		if (!pkgconf_output_file_fmt((f), __VA_ARGS__)) { \
 			pkgconf_error((client), "bomtool: Could not output to file: %s", strerror(errno)); \
 			return; \
 		} \
 	} while (0)
 
-#define OUTPUT_OR_RET_FALSE(client, f, fmt, ...) \
+#define OUTPUT_OR_RET_FALSE(client, f, ...) \
 	do { \
-		if (!pkgconf_output_file_fmt((f), (fmt), ##__VA_ARGS__)) { \
+		if (!pkgconf_output_file_fmt((f), __VA_ARGS__)) { \
 			pkgconf_error((client), "bomtool: Could not output to file: %s", strerror(errno)); \
 			return false; \
 		} \
