$NetBSD$

VSI C's native ASSERT text module cannot be selected from inside a wrapper
named assert.h.  Preserve the native assert macro semantics through its public
C RTL helper.

--- lib/assert.in.h.orig	2025-01-03 11:36:00.000000000 +0000
+++ lib/assert.in.h
@@ -24,4 +24,18 @@
 
 #@INCLUDE_NEXT@ @NEXT_ASSERT_H@
 
+#ifdef __VMS
+/* The native ASSERT text module cannot be reached through a wrapper with the
+   same name.  Reproduce its public macro using the C RTL helper.  */
+void __assert (const char *, const char *, int);
+# undef assert
+# ifdef NDEBUG
+#  define assert(expression) ((void) 0)
+# else
+#  define assert(expression) \
+     ((expression) ? (void) 0 \
+      : __assert (#expression, __FILE__, __LINE__))
+# endif
+#endif
+
 /* The definition of static_assert is copied here.  */
