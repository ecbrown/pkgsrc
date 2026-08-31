$NetBSD$

VSI C's separate compilation pass does not retain the assert macro when the
RTL text module is included through this generated wrapper.  Spell out the
native definition while preserving assert.h's re-inclusion semantics.

--- lib/assert.in.h.orig	2026-01-11 19:39:04.000000000 +0000
+++ lib/assert.in.h
@@ -23,6 +23,19 @@
 #endif
 @PRAGMA_COLUMNS@
 
-#@INCLUDE_NEXT@ @NEXT_ASSERT_H@
+#ifdef __VMS
+# undef assert
+void __assert (const char *, const char *, int);
+# ifdef NDEBUG
+#  define assert(expression) ((void) 0)
+# else
+#  define assert(expression)                                            \
+    ((expression)                                                       \
+     ? (void) 0                                                        \
+     : __assert (#expression, __FILE__, __LINE__))
+# endif
+#else
+# @INCLUDE_NEXT@ @NEXT_ASSERT_H@
+#endif
 
 /* The definition of static_assert is copied here.  */
